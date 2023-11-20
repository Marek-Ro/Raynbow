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
        
        // no intersection
        if (!its)  { 
        
        }
        // intersection
        else {

        }
        
        return Color();
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
