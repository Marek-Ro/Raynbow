#include <lightwave.hpp>

namespace lightwave
{

    class VolumeBsdf : public Bsdf
    {
        Color color;

    public:
        VolumeBsdf(const Properties &properties)
        {
            color = properties.get<Color>("color", Color(0));
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override{
                            // Sample a random ray
                            BsdfSample sample = {
                                .wi = squareToUniformSphere(rng.next2D()),
                                .weight = color
                            };
                            return sample;

        }

        BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
            BsdfEval eval = { .value = color };            
// idk if this makes sense for volumes
            // avoid negative numbers 
//            eval.value *= max(Frame::cosTheta(wi), 0) * InvPi;
            return eval;
        }

        BsdfEval evaluateAlbedo(const Point2& uv) const override {
            BsdfEval eval = { .value = color };
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
