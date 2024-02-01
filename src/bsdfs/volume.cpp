#include <lightwave.hpp>

namespace lightwave
{

    class VolumeBsdf : public Bsdf
    {
        ref<Texture> m_albedo;

    public:
        VolumeBsdf(const Properties &properties)
        {
            m_albedo = properties.get<Texture>("albedo");

        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override{
                            // Sample a random ray
                            BsdfSample sample = {
//                                .wi = squareToCosineHemisphere(rng.next2D()),
                                .wi = squareToUniformSphere(rng.next2D()),
                                .weight = m_albedo->evaluate(uv)
                            };
                            return sample;

        }

        BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
            BsdfEval eval = { .value = m_albedo->evaluate(uv) };
// idk if this makes sense for volumes
            // avoid negative numbers 
            eval.value *= abs(Frame::cosTheta(wi)) * InvPi;
            return eval;
        }

        BsdfEval evaluateAlbedo(const Point2& uv) const override {
            BsdfEval eval = { .value = m_albedo->evaluate(uv) };
            return eval;
        } 

        std::string toString() const override
        {
            return tfm::format("Volume[\n"
                               "  albedo = %s\n"
                               "]",""
                               );
        }
    };

}

REGISTER_BSDF(VolumeBsdf, "volumebsdf")
