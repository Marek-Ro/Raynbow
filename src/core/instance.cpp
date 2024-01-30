#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {

    if (m_normalMap == nullptr) {
        // Implementation from the assignments
        surf.position = m_transform->apply(surf.position);

        Vector tangent = m_transform->apply(surf.frame.tangent);
        Vector bitangent = m_transform->apply(surf.frame.bitangent);
        surf.pdf = surf.pdf / tangent.cross(bitangent).length();
        if (m_flipNormal) {
            bitangent = - bitangent;
        }

        Vector normal = tangent.cross(bitangent);
        bitangent = normal.cross(tangent);

        surf.frame.tangent = tangent.normalized();
        surf.frame.bitangent = bitangent.normalized();
        surf.frame.normal = normal.normalized();
    } else {
        surf.position = m_transform->apply(surf.position);
        // Normal mapping
        Color c = m_normalMap->evaluate(surf.uv);
        //logger(EError, "uv(%f, %f)", surf.uv.x(), surf.uv.y());
        Vector normal = Vector(c.r() * 2 - 1, c.g() * 2 - 1, c.b() * 2 - 1);
        normal = surf.frame.toWorld(normal).normalized();

        surf.frame = Frame(normal);
        Vector tangent = m_transform->apply(surf.frame.tangent);
        Vector bitangent = m_transform->apply(surf.frame.bitangent);

        normal = tangent.cross(bitangent);
        bitangent = normal.cross(tangent);

        surf.frame.tangent = tangent.normalized();
        surf.frame.bitangent = bitangent.normalized();
        surf.frame.normal = normal.normalized();
    }
}

// NO LONGER NEEDED ONLY FOR REFERENCE
// call only when you have an intersection already to check if that intersection is negated by alpha masking
// returns true if a valid intersection occurs
// returns false if the previous intersection is negated by alpha masking
bool alpha_masking_check(Texture *m_alpha_mask, Intersection *its, Sampler &rng, Ray *localRay, Shape *m_shape) {

    // we think of the alpha map as a grey scale image where all color values are equal
    // also we did not care about objects having more than 2 transparent layers before having one non-transparent layer
    // (might introduce while loop in that case)
    if (m_alpha_mask != nullptr) {
        if (m_alpha_mask->evaluate(its->uv).r() < rng.next()) {
            localRay->origin = its->position;
            // spheres are primitives that can be intersected twice
            if (m_shape->intersect(*localRay, *its, rng)) {
                if (m_alpha_mask->evaluate(its->uv).r() < rng.next()) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

float sample_distance(Sampler &rng, float density) {
    float t = (std::log(1 - rng.next())) / - density;
    return t;
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {

    // Volumes (ugly as fuck I know)
    if (this->bsdf() != nullptr) {
        VolumeType type = this->bsdf()->getVolumeType();
        if (type != NOT_A_VOLUME) {

            if (type == HETEROGENEOUS) {
                NOT_IMPLEMENTED;
            }

            Ray localRay = !m_transform ? worldRay : m_transform->inverse(worldRay);

            // so homogeneous volumes

            // note, this only properly works for konvex objects


            float distance = sample_distance(rng, this->bsdf()->getDensity());

            bool intersectionHappens = m_shape->intersect(worldRay, its, rng);
            if (intersectionHappens == false) {
                return false;
            }
            // so we know there is a volume intersection in front of us

            // checks if we are currently inside the volume (there is an intersection behind us)
            Ray reversed_ray = localRay;
            reversed_ray.direction = -localRay.direction;
            Intersection dummyIntersection = Intersection();
            // just to verify. We do not alter the intersection
            bool inside_volume = m_shape->intersect(reversed_ray, dummyIntersection, rng);

            if (inside_volume) {
                // TODO populate intersectin
                its.t = distance;
                its.position = localRay(its.t);

                return its.t > distance;
            } else {
                // we are not inside the volume, it is in front of us
                // using some rough epsilon offset to avoid self intersections
                Ray testing_if_inside_volume = Ray(localRay(distance * (1 + Epsilon)), localRay.direction);
                // this ray tests now, if the distance sampled is in the volume
                Intersection dummyIntersection = Intersection();
                if (m_shape->intersect(testing_if_inside_volume, dummyIntersection, rng) == false) {
                    // the distance sampled is outside of the volume, so we did not intersect it
                    return false;
                } else {
                    // the distance is inside the volume, so we say that our intersection is 'distance' further than hitting the volume
                    // TODO populate intersectin
                    its.t += distance;
                    its.position = localRay(its.t);
                    return true;
                }
            }
        }
    }


    // write the alpha mask in the intersection
    its.alpha_mask = m_alpha_mask.get();
    if (!m_transform) {
        // fast path, if no transform is needed
        Ray localRay = worldRay;
        if (m_shape->intersect(localRay, its, rng)) {
            its.instance = this;
            return true;
        } else {
            return false;
        }
    }

    const float previousT = its.t;

    Ray localRay = m_transform->inverse(worldRay);

    // The length of the direction vector changes by factor localRay.direction.length() / worldRay.direction.length()
    // when going from world space to local space. The localRay is not normalized but the worldRay is so the length is 1.
    float scaling = localRay.direction.length();

    localRay.direction = localRay.direction.normalized();

    // The t changes by the same factor the direction vector length changed
    // Now its.t is in according to the localRay
    its.t = previousT * scaling;

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);

    if (wasIntersected) {
        // Transform its.t back to world space
        its.t = its.t / scaling;
        its.instance = this;
        transformFrame(its);
        return true;
    } else {
        // We got no intersection so we assign the previousT, which was already correct for world space
        its.t = previousT;
        return false;
    }
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")
