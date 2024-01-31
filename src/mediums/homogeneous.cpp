#include <lightwave.hpp>

namespace lightwave
{

    class Homogeneous : public Medium
    {
        float density;

    public:
        Homogeneous(const Properties &properties)
        {
            density = properties.get<float>("density", 0);
        }

        float sample_distance(Sampler &rng) const override {
            return std::log(1 - rng.next()) / - density;;
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
