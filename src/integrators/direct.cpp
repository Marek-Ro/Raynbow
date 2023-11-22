#include <lightwave.hpp>

namespace lightwave {


class direct : public SamplingIntegrator {
    /// @brief Whether to remap the normals from [-1;1] to [0;1]
    bool remap;
    /// @brief the scene we check for intersections
    //Scene scene;

public:
    direct(const Properties &properties)
        : SamplingIntegrator(properties) {
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced by the camera model.
     * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        // Compute the intersection object
        Intersection its = scene().get()->intersect(ray,rng);
        Color first_ray_color = Color(1.0f);

        // no intersection
        if (!its) {
            first_ray_color *= m_scene.get()->evaluateBackground(ray.direction).value;
        }
        // intersection
        else {

            BsdfSample bsdfsample = its.sampleBsdf(rng);
            first_ray_color *= bsdfsample.weight;

            Ray secondary_ray = Ray(its.position, its.frame.toWorld(bsdfsample.wi).normalized());
            Intersection secondary_its = m_scene.get()->intersect(secondary_ray, rng);
            // Intersection of the secondary ray
            if (secondary_its) {
                // no light source, since we found a new intersection with an object
                first_ray_color *= secondary_its.sampleBsdf(rng).weight;
            } else {
                // Secondary ray escapes
                first_ray_color *= m_scene.get()->evaluateBackground(secondary_ray.direction).value;
            }
        }
        return first_ray_color;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "direct[\n"
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
REGISTER_INTEGRATOR(direct, "direct")
