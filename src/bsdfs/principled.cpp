#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {

        BsdfEval eval = { 
            .value = color,
            .pdf = 0,
            };

        float cos = Frame::cosTheta(wi);
        eval.value *= max(cos, 0);
        eval.value *= InvPi;
        assert(eval.value.r() >= 0 && eval.value.g() >= 0 && eval.value.b() >= 0);
        eval.pdf = cos * InvPi;        
        return eval;

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {

        BsdfSample sample = {
            .wi = squareToCosineHemisphere(rng.next2D()).normalized(),
            .weight = color
        };
        if (sample.isInvalid()) {
            return BsdfSample::invalid();
        }
        return sample;

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    // copy of roughconductor
    float alpha;
    Color color;
    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'

        Vector wm = ((wi+wo) / (wi+wo).length()).normalized();
        Color R = color;

        float D = lightwave::microfacet::evaluateGGX(alpha, wm);
        float G_wi = lightwave::microfacet::smithG1(alpha, wm, wi);
        float G_wo = lightwave::microfacet::smithG1(alpha, wm, wo); 
        
        float theta_i = Frame::cosTheta(wi); 
        float theta_o = Frame::cosTheta(wo);
        
        float div = abs((4 * theta_i * theta_o));
        if (div == 0) return BsdfEval::invalid();
        float scale = (D*G_wi*G_wo) / div;

        BsdfEval eval = {.value = R*scale};
        eval.value *= theta_i;
        return eval;
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        // copy of roughconductor
        Vector normal = lightwave::microfacet::sampleGGXVNDF(alpha, wo, rng.next2D()).normalized();
        Vector wi = reflect(wo, normal);
        Color w = color;
        BsdfSample sample = {
                                .wi = wi,
                                .weight = w * lightwave::microfacet::smithG1(alpha, normal, wi)
                            };
        if (sample.isInvalid()) {
            return BsdfSample::invalid();
        }
        return sample;

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto combination = combine(uv, wo);

        return BsdfEval { .value = (combination.diffuse.evaluate(wo, wi).value + combination.metallic.evaluate(wo, wi).value) };
        
        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto combination = combine(uv, wo);

        BsdfSample sample;
        // diffuse or metallic
        if (combination.diffuseSelectionProb > rng.next()) {
            sample = combination.diffuse.sample(wo, rng);
            if (sample.isInvalid()) {
                return BsdfSample::invalid();
            }
            sample.weight = sample.weight / combination.diffuseSelectionProb;
            return sample;
        } else {
            sample = combination.metallic.sample(wo, rng);
            if (sample.isInvalid()) {
                return BsdfSample::invalid();
            }
            sample.weight = sample.weight / (1.0 - combination.diffuseSelectionProb);
            return sample;
        }
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
