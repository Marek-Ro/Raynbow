#include <lightwave.hpp>

namespace lightwave {


class albedo: public SamplingIntegrator {
 
public:
    albedo(const Properties &properties)
        : SamplingIntegrator(properties) {
    }

    Color Li(const Ray &ray, Sampler &rng) override {

        Intersection its = m_scene->intersect(ray,rng);

        if (!its) {
            return Color(1);
        }

        return its.evaluateAlbedo().value;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "albedo[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image)
        );
    }
};

}

// this informs lightwave to use our class CameraIntegrator whenever a <integrator type="camera" /> is found in a scene file
REGISTER_INTEGRATOR(albedo, "albedo")
