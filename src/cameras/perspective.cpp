#include <lightwave.hpp>
#include <numbers>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
 * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
 * are directed in negative y direction ( @code ray.direction.y < 0 ).
 */
class Perspective : public Camera {
public:

    float fov;
    float aspect_ratio;
    float angle; // stores: tan(fov / 2)
    int width;
    int height;
    std::string fov_axis;

    Perspective(const Properties &properties)
    : Camera(properties) {
        fov = properties.get<float>("fov");
        width = properties.get<int>("width");
        height = properties.get<int>("height");
        // look into that again to be able to explain it better
        float bogenmass = fov / (float)360 * (float)std::numbers::pi;
        angle = tan(bogenmass);
        fov_axis = properties.get<std::string>("fovAxis");
        // aspect ratio depends on fov_axis
        aspect_ratio = fov_axis == "y" ? (float)width / (float)height : (float)height / (float)width;

        // hints:
        // * precompute any expensive operations here (most importantly trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {

        // Transforming normalized image coordinates to normalized camera coordinates
        float x = normalized.x() * angle;
        float y = normalized.y() * angle;
        
        // apply aspect_ratio depending on the fov_axis
        if (fov_axis == "y") {
            x = x * aspect_ratio;
        } else if (fov_axis == "x"){
            y = y * aspect_ratio;
        }

        // create direction vector, remap on imaginary plane z=1 (camera direction is [0,0,1])
        // normalize important for the ray
        Vector xy = Vector(x,y, 1.f);
        xy = xy.normalized();

        // ray with origin (0,0,0) and direction vector xy
        Ray ray = Ray(Vector(0.f, 0.f, 0.f), xy);
        ray = m_transform->apply(ray);
        

        Color weight = Color(1.0f);
        return CameraSample{
            ray, weight
        };
        
        // Example 0
        /*
        return CameraSample{
        .ray = Ray(Vector(normalized.x(), normalized.x(), 0.f),
            Vector(0.f, 0.f, 1.f)),
        .weight = Color(1.0f)}; */
    }

    std::string toString() const override {
        return tfm::format(
            "Perspective[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            indent(m_transform)
        );
    }
};

}

REGISTER_CAMERA(Perspective, "perspective")
