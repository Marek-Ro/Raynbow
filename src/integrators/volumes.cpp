#include <lightwave.hpp>

namespace lightwave {

class volumes : public SamplingIntegrator {
int depth;
bool nee;
public:
    volumes(const Properties &properties)
        : SamplingIntegrator(properties) {
        depth = properties.get<int>("depth", 1);
        nee = m_scene->hasLights();
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced by the camera model.
     * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        float volume_absorbtion = 0.9;
        bool in_volume = false;
        float volume_travel_dist = 0;   // metric is t
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

            // traveling in volumes
            if (intersection.volume_intersection && in_volume) {
                in_volume = false;
                intersection.volume_intersection = false;
//                Li -= weight * pow(volume_absorbtion, intersection.t);
                volume_travel_dist = intersection.t;
            } else if (intersection.volume_intersection) {
                // intersected a volume from outside, shoot further
                in_volume = true;
                intersection.volume_intersection = false;
                current_ray.origin = Point(current_ray.origin + current_ray.direction * Epsilon);                
                continue;
            }


            // emission after there is an intersection
            //if (current_depth == 1) 
            if (in_volume) {
                weight *= pow(volume_absorbtion, volume_travel_dist);
            }
            Li += weight * intersection.evaluateEmission();

            if (current_depth >= depth - 1) return Li;

            // next-event estimation
            if (nee) {
                LightSample light_sample =  m_scene->sampleLight(rng);
                DirectLightSample dls = light_sample.light->sampleDirect(intersection.position, rng);
                if (dls.isInvalid()) {
                    return Li;
                }
                // avoid double counting
                if (light_sample.light->canBeIntersected() == false) {
                    // check if the light source is visible
                    // therefore create a ray and shoot it in the direction of the light source to see if intersects
                    // something before that light source
                    Ray check_for_visibility_ray = Ray(intersection.position, dls.wi);

                    // now outside because volumes
                    BsdfEval eval = intersection.evaluateBsdf(dls.wi);

                    // volumes nee
                    Intersection NEEintersection = m_scene->intersect(check_for_visibility_ray, rng);
                    if (NEEintersection.volume_intersection && NEEintersection.t < dls.distance) {

                        if (in_volume) {
                            // nee to a light source outside of the volume
                            volume_travel_dist = NEEintersection.t;
                            Li += dls.weight * eval.value / light_sample.probability * weight * pow(volume_absorbtion, volume_travel_dist);
                        } else {
                            // nee into a volume (through a volume)
                        }




                    } else if (!m_scene->intersect(check_for_visibility_ray, dls.distance, rng)) {
                        // normal nee
                        
                        // the light is visible
                        
                        Li += dls.weight * eval.value / light_sample.probability * weight;
                        assert(dls.weight.r() >= 0 && dls.weight.g() >= 0 && dls.weight.b() >= 0);
                        assert(eval.value.r() >= 0 && eval.value.g() >= 0 && eval.value.b() >= 0);
                        assert(light_sample.probability > 0);
                        assert(!std::isnan(light_sample.probability));
                    }
                }
            }
            BsdfSample bsdfsample = intersection.sampleBsdf(rng);
            if (bsdfsample.isInvalid()) {
                return Li;
            }

            // update weight
            weight *= bsdfsample.weight;
            assert_finite(weight, {
                logger(EError, "bsdf sample weight = %s", bsdfsample.weight);
                logger(EError, "offending BSDF %s", intersection.instance->bsdf());
            });
            // construct next ray
            current_ray.origin = intersection.position;
            current_ray.direction = bsdfsample.wi;
            //current_ray = ray;
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
REGISTER_INTEGRATOR(volumes, "volumes")
