#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave
{

    class RoughConductor : public Bsdf
    {
        ref<Texture> m_reflectance;
        ref<Texture> m_roughness;

    public:
        RoughConductor(const Properties &properties)
        {
            m_reflectance = properties.get<Texture>("reflectance");
            m_roughness = properties.get<Texture>("roughness");
        }

        BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                          const Vector &wi) const override
        {
            // Using the squared roughness parameter results in a more gradual
            // transition from specular to rough. For numerical stability, we avoid
            // extremely specular distributions (alpha values below 10^-3)
            const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

            // hints:
            // * the microfacet normal can be computed from `wi' and `wo'

            Vector wm = (wi + wo).normalized();

            // parameters for the formula
            Color R = m_reflectance->evaluate(uv);
            float D = lightwave::microfacet::evaluateGGX(alpha, wm);
            float G_wi = lightwave::microfacet::smithG1(alpha, wm, wi);
            float G_wo = lightwave::microfacet::smithG1(alpha, wm, wo);

            // incoming and outgoing angle
            float theta_i = Frame::cosTheta(wi);
            float theta_o = Frame::cosTheta(wo);

            // formula from the assignment
            float scale = (D * G_wi * G_wo) / (4 * theta_i * theta_o);
            BsdfEval eval = {.value = R * scale};

            eval.value *= theta_i;
            return eval;
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override
        {
            const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

            // hints:
            // * do not forget to cancel out as many terms from your equations as possible!
            //   (the resulting sample weight is only a product of two factors)
            Vector normal = lightwave::microfacet::sampleGGXVNDF(alpha, wo, rng.next2D()).normalized();
            Vector wi = reflect(wo, normal);

            BsdfSample sample = {
                .wi = wi,
                // simplify the formula by cancelling out as many terms as possible
                .weight = m_reflectance->evaluate(uv) * lightwave::microfacet::smithG1(alpha, normal, wi)
                };
            if (sample.isInvalid())
            {
                return BsdfSample::invalid();
            }
            return sample;
        }

        std::string toString() const override
        {
            return tfm::format("RoughConductor[\n"
                               "  reflectance = %s,\n"
                               "  roughness = %s\n"
                               "]",
                               indent(m_reflectance), indent(m_roughness));
        }
    };

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
