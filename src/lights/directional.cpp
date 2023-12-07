#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
public:
    Color intensity;
    Vector direction;
    
    DirectionalLight(const Properties &properties) {
        direction = properties.get<Vector>("direction");
        intensity = properties.get<Color>("intensity");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        
        DirectLightSample d = DirectLightSample {
            .wi = direction.normalized(),
            .weight = intensity / Pi,
            .distance = Infinity,
        };
        return d;
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("DirectionalLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
