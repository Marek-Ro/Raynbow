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
class Thinlens : public Camera {
public:

    float fov;
    float aspect_ratio;
    float BC; // stores: tan(fov / 2)
    int width;
    int height;
    std::string fov_axis;

// Thinlens
//    float depth_of_field;
    float focalDistance;
    float lensRadius;

    Thinlens(const Properties &properties)
    : Camera(properties) {
        fov = properties.get<float>("fov");
        width = properties.get<int>("width");
        height = properties.get<int>("height");
        // From degree to radian 
        float radian = fov / (float)360 * 2 * (float)std::numbers::pi;
        BC = tan(radian / 2);
        fov_axis = properties.get<std::string>("fovAxis");
        // aspect ratio depends on fov_axis
        aspect_ratio = fov_axis == "y" ? (float)width / (float)height : (float)height / (float)width;

        lensRadius = 0.1;
        focalDistance = 0.01;
        // hints:
        // * precompute any expensive operations here (most importantly trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {

        // Transforming normalized image coordinates to normalized camera coordinates
        float x = normalized.x() * BC;
        float y = normalized.y() * BC;
        
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



        // pbrt thinlens here

        // x and y are in [0, 1) already

        // Point on the lens
        Vector2 pLens = lensRadius * sampleUniformDiskConcentric(Vector2(normalized.x(), normalized.y()));

        // compute point on the plane of focus
        float ft = focalDistance / ray.direction.z();
        Point pFocus = ray(ft);

        // update ray for effect of lens
        ray.origin = Point(pLens.x(), pLens.y(), 0);
        ray.direction = (pFocus - ray.origin).normalized();





        // finally transform to world space
        ray = m_transform->apply(ray).normalized();

        Color weight = Color(1.0f);
        return CameraSample{
            ray, weight
        };
    }

    Vector2 sampleUniformDiskConcentric(Vector2 u) const {
        
        // remap u from [0, 1] to [-1, 1]
        Vector2 uOffset = (2 * u) - Vector2(1, 1);
        if (uOffset.x() == 0 && uOffset.y() == 0) {
            return uOffset;
        }

        // apply concentric mapping to the point
        float theta, r;
        if (std::abs(uOffset.x()) > std::abs(uOffset.y())) {
            r = uOffset.x();
            theta = (Pi / 4) * (uOffset.y() / uOffset.x());
        } else {
            r = uOffset.y();
            theta = (Pi / 2) - (Pi / 4) * (uOffset.x() / uOffset.y());
        }

        return r * Vector2(std::cos(theta), std::sin(theta));
    }

    std::string toString() const override {
        return tfm::format(
            "Thinlens[\n"
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

REGISTER_CAMERA(Thinlens, "thinlens")
