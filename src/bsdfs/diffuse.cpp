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

            float cos = Frame::cosTheta(wi);
//            cos = wi.z() / sqrt(wi.x() * wi.x() + wi.y() * wi.y());
            eval.value *= cos;
//            if (cos <= 0) logger(EError, "%f", cos);
//            assert(eval.value.r() >= 0 && eval.value.g() >= 0 && eval.value.b() >= 0);
//            assert(wi.z() > 0);
            if (eval.value.r() < 0) eval.value.r() = 0;
            if (eval.value.g() < 0) eval.value.g() = 0;
            if (eval.value.b() < 0) eval.value.b() = 0;
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
