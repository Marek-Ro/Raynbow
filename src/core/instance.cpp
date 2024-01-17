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
        normal = m_transform->apply(normal).normalized();

        surf.frame = Frame(normal);
    }
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {
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
