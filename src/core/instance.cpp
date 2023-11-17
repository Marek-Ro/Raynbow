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
    //Vector previous_hitpoint_vector = its.position - Point(0);
    //Ray localRay;
    //float previous_hitpoint_x = previous_hitpoint_vector.x();
    //Vector new_hitpoint_vector = m_transform->apply(previous_hitpoint_vector);
    //float new_hitpoint_x = new_hitpoint_vector.x();
    Ray localRay = m_transform->inverse(worldRay);
    localRay.direction = localRay.direction.normalized();
    //localRay = localRay.normalized();
    //float t_factor = previous_hitpoint_x / new_hitpoint_x;
    //its.t = previousT * t_factor;


    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does its.t need to change?

// P = O + tD
// P - 0 = tD

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        //Point transformed_hitpoint = m_transform->inverse(worldRay(its.t));
        Point transformed_hitpoint = m_transform->apply(localRay(its.t));
        its.t = (transformed_hitpoint - worldRay.origin).x() / worldRay.direction.x();
        //float new_t = p_o.y() / worldRay.direction.y();
        //its.t = new_t;
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
