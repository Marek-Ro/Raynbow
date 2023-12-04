#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
public:
    Color weight;
    Point position;
    
    PointLight(const Properties &properties) {
        position = properties.get<Point>("position");        
        weight = properties.get<Color>("power");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        
        DirectLightSample d = DirectLightSample {
            .wi = origin - position,
            .weight = weight * Inv4Pi,
            .distance = (origin - position).length(),
        };
        return d;
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
