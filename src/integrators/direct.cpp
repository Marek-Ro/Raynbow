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
        Intersection its = m_scene->intersect(ray,rng);
        Color first_ray_color = Color(1.0f);

        // no intersection
        if (!its) {
            first_ray_color *= m_scene->evaluateBackground(ray.direction).value;
        }
        // intersection
        else {

            // Wenn emission returnen wir, weil wir würden ja keinen Schatten auf ne Lampe werfen
            if (its.evaluateEmission() != Color(0)) {
                return first_ray_color * its.evaluateEmission();
            }

            BsdfSample bsdfsample = its.sampleBsdf(rng);

            if (bsdfsample.isInvalid()) {
                return its.evaluateEmission();
            }

            first_ray_color *= bsdfsample.weight;

            Ray secondary_ray = Ray(its.position, bsdfsample.wi.normalized());
            Intersection secondary_its = m_scene->intersect(secondary_ray, rng);
            // Intersection of the secondary ray
            if (secondary_its) {
                // no light source, since we found a new intersection with an object
                return first_ray_color * secondary_its.evaluateEmission();
            } else {
                // Secondary ray escapes
                return first_ray_color * (Color(1.0f) * m_scene->evaluateBackground(secondary_ray.direction).value);
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
