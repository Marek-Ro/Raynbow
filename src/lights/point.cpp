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
        return DirectLightSample {
            .wi = (position - origin).normalized(),
            // flux to intensity
            .weight = (weight * Inv4Pi) / ((origin - position).length() * (origin - position).length()),
            //.weight = (weight * Inv4Pi) / (sqr((origin - position).length())),
            .distance = (origin - position).length(),
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
