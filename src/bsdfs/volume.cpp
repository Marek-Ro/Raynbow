#include <lightwave.hpp>

namespace lightwave
{

    class VolumeBsdf : public Bsdf
    {
        Color color;
        float density;
        VolumeType volume_type;

    public:
        VolumeBsdf(const Properties &properties)
        {
            density = properties.get<float>("density", 0);
            color = properties.get<Color>("color", Color(0));
            volume_type = properties.get<VolumeType>("type", HOMOGENEOUS);
        }

        Vector sample_random_angle(Sampler &rng) const {
            // random values between -1 and 1
            float r1 = (rng.next()*2)-1;
            float r2 = (rng.next()*2)-1;
            float r3 = (rng.next()*2)-1;
            return Vector(r1,r2,r3).normalized();
        }


        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override{
                            // Sample a random ray
                            BsdfSample sample = {
                                .wi = sample_random_angle(rng),
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

        VolumeType getVolumeType() const override { return volume_type; }

        // Volumes only, otherwise Color(0)
        Color getColor() const override { return color; }

        // Volumes only, otherwise 0
        float getDensity() const override { return density; }

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
