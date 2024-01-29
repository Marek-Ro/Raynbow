#include <lightwave.hpp>

namespace lightwave {

class path_volume : public SamplingIntegrator {
int depth;
bool nee;
float density;
Color color;
public:
    path_volume(const Properties &properties)
        : SamplingIntegrator(properties) {
        depth = properties.get<int>("depth", 1);
        nee = m_scene->hasLights();
        density = 0.5;
        color = Color(0);
    }

    float sample_distance(Sampler &rng, float density) {
        float t = (std::log(1 - rng.next())) / - density;
        return t;
    }

    Vector sample_random_angle(Sampler &rng) {
        // random values between -1 and 1
        float r1 = (rng.next()*2)-1;
        float r2 = (rng.next()*2)-1;
        float r3 = (rng.next()*2)-1;
        return Vector(r1,r2,r3).normalized();
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

            float sampled_distance = sample_distance(rng, density);
            if (intersection.t > sampled_distance) {
                // scatter
                

                // bsdf
                weight *= color;
                // random angle
                current_ray.direction = sample_random_angle(rng);
                current_ray.origin = current_ray(sampled_distance);

                // NEE fpr volumes
                float nee_distance_sample = sample_distance(rng, density);
                
                
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

                    if (!m_scene->intersect(check_for_visibility_ray, dls.distance, rng) && 
                        // also check if the fog eats it
                        dls.distance < nee_distance_sample)
                    {
                        // the light is visible
                        BsdfEval eval = intersection.evaluateBsdf(dls.wi);
                        Li += dls.weight * eval.value / light_sample.probability * weight;
                        assert(dls.weight.r() >= 0 && dls.weight.g() >= 0 && dls.weight.b() >= 0);
                        assert(eval.value.r() >= 0 && eval.value.g() >= 0 && eval.value.b() >= 0);
                        assert(light_sample.probability > 0);
                        assert(!std::isnan(light_sample.probability));
                    }


                continue;
            }
            // take the intersection as usual

            // handle escaping rays
            if (!intersection) {
                Li += weight * m_scene->evaluateBackground(current_ray.direction).value;
                return Li;
            }
            // emission after there is an intersection
            //if (current_depth == 1) 
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

                    if (!m_scene->intersect(check_for_visibility_ray, dls.distance, rng)) {
                        // the light is visible
                        BsdfEval eval = intersection.evaluateBsdf(dls.wi);
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
    }}

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
REGISTER_INTEGRATOR(path_volume, "path_volume")
