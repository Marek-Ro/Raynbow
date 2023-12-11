#include <lightwave.hpp>

namespace lightwave {

class pathtracer : public SamplingIntegrator {
int depth;
public:
    pathtracer(const Properties &properties)
        : SamplingIntegrator(properties) {
        depth = properties.get<int>("depth", 1);
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced by the camera model.
     * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Ray current_ray = ray;
        Color Li = Color(0);
        Color weight = Color(1);
        for (int current_depth = 0; current_depth < depth; current_depth++) {
            Intersection intersection = m_scene->intersect(current_ray, rng);
            // handle escaping rays
            if (!intersection) {
                Li += weight * m_scene->evaluateBackground(current_ray.direction).value;
                return Li;
            }
            // emission after there is an intersection
            Li += weight * intersection.evaluateEmission();
            // next-event estimation
            if (m_scene->hasLights()) {
                LightSample light_sample =  m_scene->sampleLight(rng);
                DirectLightSample dls = light_sample.light->sampleDirect(intersection.position, rng);
                if (dls.isInvalid()) {
                    return Color(0);
                }
                // avoid double counting
                if (light_sample.light->canBeIntersected() == false) {
                    // check if the light source is visible
                    // therefore create a ray and shoot it in the direction of the light source to see if intersects
                    // something before that light source
                    Ray check_for_visibility_ray = Ray(intersection.position, dls.wi);

                    if (!m_scene->intersect(check_for_visibility_ray, dls.distance, rng)) {
                        // the light is visible
                        BsdfEval eval = intersection.evaluateBsdf(dls.wi);
                        Li += dls.weight * eval.value / light_sample.probability;
                        assert(light_sample.probability > 0);
                        assert(!std::isnan(light_sample.probability));
                    }
                }
            }
            BsdfSample bsdfsample = intersection.sampleBsdf(rng);
            if (bsdfsample.isInvalid()) {
                return Color(0);
            }
            // contribution of the intersection point
            Li += bsdfsample.weight * weight;
            // update weight
            weight *= bsdfsample.weight;
            // construct next ray
            current_ray = Ray(intersection.position, bsdfsample.wi);
        }
        return Li;
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
REGISTER_INTEGRATOR(pathtracer, "pathtracer")
