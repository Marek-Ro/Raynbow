#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));


        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'

        Vector wm = (wi+wo) / (wi+wo).length();
        Color R = m_reflectance->evaluate(uv);

        // TODO error wenn lightwave::microfacet:: nicht da steht
        float D = lightwave::microfacet::evaluateGGX(alpha, wm);
        float G_wi = lightwave::microfacet::smithG1(alpha, wm, wi);
        float G_wo = lightwave::microfacet::smithG1(alpha, wm, wo); 
        
        // incoming and outgoing angle 
        // TODO weiß nicht ob cosTheta hier richtig ist und ob dann unten cos genommen werden muss
        float theta_i = Frame::cosTheta(wi); 
        float theta_o = Frame::cosTheta(wo);
        
        float scale = (D*G_wi*G_wo) / (4 * cos(theta_i) * cos(theta_o));

        BsdfEval eval = {.value = R*scale};
        return eval;

    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        
        // hints:
        // * do not forget to cancel out as many terms from your equations as possible!
        //   (the resulting sample weight is only a product of two factors)
        Vector normal = lightwave::microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());

        BsdfSample sample = {
                                .wi = normal,
                                .weight = m_reflectance->evaluate(uv)
                            };
        return sample;
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
