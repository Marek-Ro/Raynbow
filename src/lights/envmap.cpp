#include <lightwave.hpp>

namespace lightwave
{

    class EnvironmentMap final : public BackgroundLight
    {
        /// @brief The texture to use as background
        ref<Texture> m_texture;

        /// @brief An optional transform from local-to-world space
        ref<Transform> m_transform;

    public:
        EnvironmentMap(const Properties &properties)
        {
            m_texture = properties.getChild<Texture>();
            m_transform = properties.getOptionalChild<Transform>();
        }

        BackgroundLightEval evaluate(const Vector &direction) const override
        {
            Vector direction2 = direction.normalized();

            // transform from world to local coordinates
            if (m_transform)
            {
                direction2 = m_transform->inverse(direction).normalized();
            }

            // convert from cartesian to spherical coordinates
            float theta = safe_acos(direction2.y());
            float phi = atan2(-direction2.z(), direction2.x());
            // map the spherical coordinates to [0;1]
            Vector2 warped = Vector2((phi + Pi) * Inv2Pi, theta * InvPi);

            return {
                .value = m_texture->evaluate(warped),
            };
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            Vector direction = squareToUniformSphere(rng.next2D());
            auto E = evaluate(direction);

            // implement better importance sampling here, if you ever need it
            // (useful for environment maps with bright tiny light sources, like the
            // sun for example)

            return {
                .wi = direction,
                .weight = E.value / Inv4Pi,
                .distance = Infinity,
            };
        }

        std::string toString() const override
        {
            return tfm::format("EnvironmentMap[\n"
                               "  texture = %s,\n"
                               "  transform = %s\n"
                               "]",
                               indent(m_texture), indent(m_transform));
        }
    };

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
