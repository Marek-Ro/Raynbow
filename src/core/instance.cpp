#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>
#include <lightwave/warp.hpp>

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

void populateVolumeIntersection(Intersection &its, Ray &ray, float new_t, Sampler &rng, float scaling) {
    // back to world space
    its.t = new_t / scaling;
    its.position = ray(its.t);
    //its.frame = Frame(squareToUniformSphere(rng.next2D()));
    its.frame = Frame(ray.direction);
    its.pdf = 0;
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {

    // better volumes (m_medium is a ref, get() to get a pointer out of it)
    if (m_medium) {

        Ray localRay = !m_transform ? worldRay : m_transform->inverse(worldRay);
        // see below
        float scaling = localRay.direction.length();
        localRay.direction = localRay.direction.normalized();

        Intersection its_incoming_state = its;


        its.t *= scaling;


        bool intersectionHappens = m_shape->intersect(localRay, its, rng);
        if (intersectionHappens == false) {
            return false;
        }

        bool inside_volume = its.frame.normal.dot(localRay.direction) < 0 ? false : true;

        float distance = m_medium->sample_distance(rng);

        if (inside_volume) {
            if (its.t < distance) {
                // ray escapes
                its = its_incoming_state;
                return false;
            } else {
                // populate intersection
                its.instance = this;
                populateVolumeIntersection(its, localRay, distance, rng, scaling);
                
                transformFrame(its);
                return true;
            }
        } else {
            // find the back side of the mesh (only konvex meshes so far)
            Ray backsideRay = localRay;
            backsideRay.origin = its.position;
            Intersection backface_Intersection = Intersection();
            m_shape->intersect(backsideRay, backface_Intersection, rng);
            if (backface_Intersection.t < distance) {
                // ray escapes
                its = its_incoming_state;
                return false;
            } else {
                its.instance = this;
                // case where we start outside of the volume, so the hitpoint
                // is the sampled distance + the distance to get to the volume (its.t)
                populateVolumeIntersection(its, localRay, its.t + distance, rng, scaling);

                transformFrame(its);
                return true;
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

    const float previous_t = its.t;

    Ray localRay = m_transform->inverse(worldRay);

    // The length of the direction vector changes by factor localRay.direction.length() / worldRay.direction.length()
    // when going from world space to local space. The localRay is not normalized but the worldRay is so the length is 1.
    float scaling = localRay.direction.length();

    localRay.direction = localRay.direction.normalized();

    // The t changes by the same factor the direction vector length changed
    // Now its.t is in according to the localRay
    its.t = previous_t * scaling;

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);

    if (wasIntersected) {
        // Transform its.t back to world space
        its.t = its.t / scaling;
        its.instance = this;
        transformFrame(its);
        return true;
    } else {
        // We got no intersection so we assign the previousT, which was already correct for world space
        its.t = previous_t;
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
