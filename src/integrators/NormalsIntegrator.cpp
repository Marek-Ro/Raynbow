#include <lightwave.hpp>

namespace lightwave {


class NormalsIntegrator : public SamplingIntegrator {
    /// @brief Whether to remap the normals from [-1;1] to [0;1]
    bool remap;
    /// @brief the scene we check for intersections
    //Scene scene;

public:
    NormalsIntegrator(const Properties &properties)
        : SamplingIntegrator(properties)/*, scene(Scene(properties))*/ {
        //scene = samplingIntegrator.scene();
        //scene = Scene(properties);
        // Task 1.2.1 "The normal integrator takes a single parameter remap"
        try {
            remap = properties.get<bool>("remap");
        }
        catch (...) {
            remap = true;
        }
    }



    /**
     * @brief The job of an integrator is to return a color for a ray produced by the camera model.
     * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        // Intersect the ray against the scene and get the intersection information
        // If an intersection occurs, store the normal at that intersection or 0 if no intersection
        Vector normal = ray.direction;
        //bool test = scene().get()->intersect(ray, 10, rng);
        if (scene().get()->intersect(ray, 10, rng)) { //TODO what should tmax be?
            normal = scene().get()->intersect(ray,rng).frame.normal;
        }
        else {
            normal = Vector(0);
        }
        
        // Map values from [-1;1] to [0;1] -> add 1 and divide by 2 
        if (remap) {
            normal[0] = (normal[0] + 1) / 2;
            normal[1] = (normal[1] + 1) / 2;
            normal[2] = (normal[2] + 1) / 2;
        }
        // Return the (potentially remapped) normal.
        return Color(normal[0], normal[1], normal[2]);
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "NormalsIntegrator[\n"
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
REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
