#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {

    surf.position = m_transform->apply(surf.position);

    Vector tangent = m_transform->apply(surf.frame.tangent);
    //Vector tangent = surf.frame.tangent;
    Vector bitangent = surf.frame.bitangent;
    Vector normal = surf.frame.normal;

    Vector new_bitangent = tangent.cross(normal);

    if (m_flipNormal) {
        surf.frame.bitangent = - new_bitangent.normalized();
    }


    //Vector new_normal = new_bitangent.cross(tangent);
    Vector new_normal = m_transform->inverse(normal);
    //surf.frame.bitangent = new_bitangent.normalized();
    
    //surf.frame.normal = m_transform->apply(new_normal).normalized();
    surf.frame.normal = new_normal.normalized();



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
    Vector previous_hitpoint_vector = its.position - Point(0);
    Ray localRay;
    float previous_hitpoint_x = previous_hitpoint_vector.x();
    Vector new_hitpoint_vector = m_transform->apply(previous_hitpoint_vector);
    float new_hitpoint_x = new_hitpoint_vector.x();
    localRay = m_transform->inverse(worldRay);
    localRay = localRay.normalized();
    float t_factor = previous_hitpoint_x / new_hitpoint_x;
    //its.t = previousT * t_factor;


    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does its.t need to change?

// P = O + tD
// P - 0 = tD

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        Point transformed_hitpoint = m_transform->inverse(worldRay(its.t));


        Vector p_o = transformed_hitpoint - localRay.origin;
        float new_t = p_o.x() / localRay.direction.x();
        its.t = new_t;
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
