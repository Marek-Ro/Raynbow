#include <lightwave.hpp>

namespace lightwave {


class direct : public SamplingIntegrator {
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
        // start with wite color 
        Color ray_color = Color(1.0f);

        // no intersection
        if (!its) {
            // background color
            ray_color *= m_scene->evaluateBackground(ray.direction).value;
        }
        // intersection
        else {
            if (its.evaluateEmission() != Color(0)) {
                // return because a shadow on a light emitting object doesn't make sense
                return ray_color * its.evaluateEmission();
            }

            BsdfSample bsdfsample = its.sampleBsdf(rng);

            if (bsdfsample.isInvalid()) {
                return Color(0);
            }
            // scale our color according to the weight of the intersection 
            ray_color *= bsdfsample.weight;

            if (m_scene->hasLights()) {
                LightSample light_sample =  m_scene->sampleLight(rng);
                DirectLightSample d = light_sample.light->sampleDirect(its.position, rng);

                // avoid double counting
                if (light_sample.light->canBeIntersected() == false) {
                    // check if the light source is visible
                    // therefore create a ray and shoot it in the direction of the light source to see if intersects
                    // something before that light source
                    Ray check_for_visibility_ray = Ray(its.position, d.wi);
                    Intersection check_for_visibility_its = m_scene->intersect(check_for_visibility_ray, rng);

                    if (check_for_visibility_its.t >= d.distance) {
                        // the light is visible
                        //ray_color += check_for_visibility_its.evaluateBsdf(d.wi).value;

                        ray_color += (d.weight * light_sample.probability) * m_scene->evaluateBackground(d.wi).value;
                        return ray_color;
                    }
                }
            }

            // create the secondary ray 
            Ray secondary_ray = Ray(its.position, bsdfsample.wi.normalized());
            Intersection secondary_its = m_scene->intersect(secondary_ray, rng);
            // Intersection of the secondary ray
            if (secondary_its) {
                // if the secondary ray hits a backface, do not return the emission from the face, 
                // but act as if it was no light source
                if (secondary_its.frame.normal.dot(secondary_its.wo) < 0) {
                    return Color(0);
                }

                // no light source, since we found a new intersection with an object
                return ray_color * secondary_its.evaluateEmission();
            } else {
                // Secondary ray escapes
                return ray_color * (Color(1.0f) * m_scene->evaluateBackground(secondary_ray.direction).value);
            }
        }
        return ray_color;
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
