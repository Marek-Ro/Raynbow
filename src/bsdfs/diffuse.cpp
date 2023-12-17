#include <lightwave.hpp>

namespace lightwave
{

    class Diffuse : public Bsdf
    {
        ref<Texture> m_albedo;

    public:
        Diffuse(const Properties &properties)
        {
            m_albedo = properties.get<Texture>("albedo");
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override{
                            // Sample a random ray
                            BsdfSample sample2 = {
                                .wi = squareToCosineHemisphere(rng.next2D()).normalized(),
                                .weight = m_albedo->evaluate(uv)
                            };
                            if (sample2.isInvalid()) {
                                return BsdfSample::invalid();
                            }
                            return sample2;

        }

        BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
            BsdfEval eval = { .value = m_albedo->evaluate(uv) };
            // avoid negative numbers 
            eval.value *= max(Frame::cosTheta(wi), 0) * InvPi;
            return eval;
        }

        BsdfEval evaluateAlbedo(const Point2& uv) const override {
            BsdfEval eval = { .value = m_albedo->evaluate(uv) };
            return eval;
        } 

        std::string toString() const override
        {
            return tfm::format("Diffuse[\n"
                               "  albedo = %s\n"
                               "]",
                               indent(m_albedo));
        }
    };

}

REGISTER_BSDF(Diffuse, "diffuse")
