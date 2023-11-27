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

        if (m_transform) {
            Vector direction = m_transform->inverse(direction);
        }
        // to spherical
        // TODO can be simplified
        float theta, phi;
        theta = Deg2Rad * safe_acos(direction.z() / direction.length());
        if (direction.y() > 0) {
             phi = Deg2Rad*safe_acos(direction.x() / safe_sqrt(direction.x() * direction.x() + direction.y() * direction.y()));
        }
        else {
             phi = -Deg2Rad*safe_acos(direction.x() / safe_sqrt(direction.x() * direction.x() + direction.y() * direction.y()));
        }
        // to [0;1]
        phi = phi + Pi / (2* Pi);
        theta = theta / Pi;
        warped.x() = phi;
        warped.y() = theta;

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
