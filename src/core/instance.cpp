#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {

    m_transform->apply(surf.position);

    if (m_flipNormal) {
        Vector tangent = surf.frame.tangent;
        Vector bitangent = surf.frame.bitangent;
        Vector normal = surf.frame.normal;

        Vector new_bitangent = tangent.cross(normal);
        Vector new_normal = new_bitangent.cross(tangent);

        surf.frame.bitangent = new_bitangent;
        surf.frame.normal = new_normal;
        
    }


    // hints:
    // * transform the hitpoint and frame here
    // * if m_flipNormal is true, flip the direction of the bitangent (which in effect flips the normal)
    // * make sure that the frame is orthonormal (you are free to change the bitangent for this, but keep
    //   the direction of the transformed tangent the same)
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
    Ray localRay;

    localRay = m_transform->inverse(worldRay);
    localRay = localRay.normalized();
    Point transformed_hitpoint = m_transform->inverse(worldRay(its.t));
        

    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does its.t need to change?

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        // hint: how does its.t need to change?

        its.instance = this;
        transformFrame(its);
        return true;
    } else {
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
