#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {

    Vector t = m_transform->apply(surf.frame.tangent).normalized();
    Vector b = m_transform->apply(surf.frame.bitangent).normalized();

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

    Ray localRay = m_transform->inverse(worldRay).normalized();
    //localRay.direction = localRay.direction.normalized();

    
    Point h_trans = m_transform->inverse(its.position);
    its.t = (h_trans - localRay.origin).length();
    const bool wasIntersected = m_shape->intersect(localRay, its, rng);

    
    if (wasIntersected) {
        Point transformed_hitpoint = m_transform->apply(localRay(its.t));
        //its.t = compute_t(worldRay.origin, worldRay.direction, transformed_hitpoint);
        its.t = (transformed_hitpoint - worldRay.origin).length();
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
