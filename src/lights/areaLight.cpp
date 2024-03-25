#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {
public:
    ref<Instance> instance;
    float spotlight_factor;

    AreaLight(const Properties &properties) {
        instance = properties.getChild<Instance>("Instance");
        spotlight_factor = properties.get<float>("spotlight_factor", 1);
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {

        AreaSample area_sample = instance.get()->sampleArea(rng);
        Vector wi = (area_sample.position - origin).normalized();
        float cos_theta = std::pow(Frame::absCosTheta(area_sample.frame.toLocal(wi)), 1);
        float length = (origin - area_sample.position).length();
        float scalar = (area_sample.pdf * (sqr(length)/ cos_theta));

        return DirectLightSample {
            .wi = wi,
            .weight = (instance.get()->emission()->evaluate(area_sample.uv, area_sample.frame.toLocal(-wi))).value / scalar,
            .distance = (origin - area_sample.position).length(),
        };
        }
    

    bool canBeIntersected() const override { 
        return false; 
        }

    std::string toString() const override {
        return tfm::format("AreaLight[\n"
                           "]");
    }
};

} // namespace lightwave


REGISTER_LIGHT(AreaLight, "area")
