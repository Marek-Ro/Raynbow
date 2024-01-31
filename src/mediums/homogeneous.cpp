#include <lightwave.hpp>

namespace lightwave
{

    class Homogeneous : public Medium
    {
        float density;

    public:
        Homogeneous(const Properties &properties)
        {
            // 0 is bad
            density = properties.get<float>("density", 1);
        }

        float sample_distance(Sampler &rng) const override {
            float res = std::log(1 - rng.next()) / - density;
            return res;
        };

        std::string toString() const override
        {
            return tfm::format("HomogeneousVolume[\n"
                               "  density = %f\n"
                               "]", density
                               );
        }
    };

}

REGISTER_MEDIUM(Homogeneous, "homogeneous")
