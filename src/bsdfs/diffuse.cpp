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
                            return sample2;

        }

        BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {

            BsdfEval eval = { .value = m_albedo->evaluate(uv) };

            eval.value *= Frame::cosTheta(wi);
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
