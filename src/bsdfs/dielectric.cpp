#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        float ior = m_ior->scalar(uv);
        float cosThetaWo = Frame::cosTheta(wo);
        // inside vs outside
        float eta = cosThetaWo > 0 ? ior : 1.0f / ior;
        float fresnel = fresnelDielectric(cosThetaWo, eta);

        BsdfSample s;

        if (rng.next() < fresnel) {
            // reflect 
            s.wi = reflect(wo, Vector(0, 0, 1));
            s.weight = m_reflectance->evaluate(uv);
        } else {
            // refract 

            // check whether you come from inside or from outside and change the normal accordingly
            if (cosThetaWo > 0) {
                s.wi = refract(wo, Vector(0, 0, 1), eta);
            } else {
                s.wi = refract(wo, Vector(0, 0, -1), eta);
            }
            s.weight = m_transmittance->evaluate(uv) / sqr(eta);
        }
        if (s.isInvalid()) {
            return BsdfSample::invalid();
        }

        return s;
    }

    BsdfEval evaluateAlbedo(const Point2& uv) const override {
        BsdfEval eval = { .value = Color(1) };
        return eval;
    } 


    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
