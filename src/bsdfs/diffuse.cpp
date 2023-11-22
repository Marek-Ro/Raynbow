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
                            Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();

                            BsdfSample sample2 = {
                                .wi = wi,
                                .weight = m_albedo->evaluate(uv)
                            };
                            return sample2;

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
