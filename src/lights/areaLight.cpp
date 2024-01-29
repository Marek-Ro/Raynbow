#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {
public:
    Color weight;
    //Point position;
    ref<Instance> instance;
    //ref<Texture> m_emission;
    
    AreaLight(const Properties &properties) {
        instance = properties.getChild<Instance>(true);
        //position = properties.get<Point>("position");        
        //weight = properties.get<Color>("power");
        //m_emission = properties.get<Texture>("emission");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // get random point on shape -> implement in shape itself
        // wi = randomPoint-origin.normalized
        // distance = origin - randomPoint).length 
        // weight = (evaluate emission?) / probability_of_sampling_that_point * (length² / cos  theta)
        
        // cos theta angle between normal and incoming (wi)?

        // sample a random point on the shape 
        AreaSample area_sample = instance.get()->sampleArea(rng);
        float pdf = area_sample.pdf;
        Point random_point_on_light = area_sample.position;
        Point2 uv = area_sample.uv;
        Vector wi = (random_point_on_light - origin).normalized();
        float cos_theta = area_sample.frame.cosTheta(wi);
        
        float length = (origin - random_point_on_light).length();
        
        float scalar = (pdf * (sqr(length)/ cos_theta));
        return DirectLightSample {
            .wi = wi,
            .weight = (instance.get()->emission()->evaluate(uv, -wi)).value / scalar,
            .distance = (origin - random_point_on_light).length(),
        };
                                   }
    

    bool canBeIntersected() const override { 
        return true; 
        }

    std::string toString() const override {
        return tfm::format("AreaLight[\n"
                           "]");
    }
};

} // namespace lightwave


REGISTER_LIGHT(AreaLight, "area")
