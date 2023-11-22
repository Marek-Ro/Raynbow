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
                        // probability of that sampled ray
                        float p_for_this_sampled_ray = cosineHemispherePdf(wi);

                        BsdfSample sample2 = {
                            .wi = wi,
                            .weight = ((m_albedo.get()->evaluate(uv) / Pi) * p_for_this_sampled_ray) * wi.dot(Vector(0,0,1))
//                                .weight = m_albedo.get()->evaluate(uv)
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
