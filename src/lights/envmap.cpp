#include <lightwave.hpp>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    ref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

public:
    EnvironmentMap(const Properties &properties) {
        m_texture   = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
    }

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector2 warped = Vector2(0, 0);
        // hints:
        // * if (m_transform) { transform direction vector from world to local
        // coordinates }
        // * find the corresponding pixel coordinate for the given local
        // direction
        Vector direction2 = direction;
        if (m_transform) {
            direction2 = m_transform->inverse(direction).normalized();
            
        }
        float theta, phi;
        theta = acos(direction2.y());
        phi = - atan2(direction2.x(), direction2.z());

//        if (direction2.z() < 0) {
//            phi = -1 * phi;
//        }

        // to [0;1]
        //phi = 1 - phi;
        warped.x() = (phi + Pi) * (1 / (2 * Pi));
        warped.y() = theta * (1 / Pi);
        return {
            .value = m_texture->evaluate(warped),
        };
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = squareToUniformSphere(rng.next2D());
        auto E           = evaluate(direction);

        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)

        return {
            .wi     = direction,
            .weight = E.value / Inv4Pi,
            .distance = Infinity,
        };
    }

    std::string toString() const override {
        return tfm::format("EnvironmentMap[\n"
                           "  texture = %s,\n"
                           "  transform = %s\n"
                           "]",
                           indent(m_texture), indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
