#include <lightwave.hpp>
namespace lightwave {
class Sphere : public Shape {
public:
Sphere(const Properties &properties) {
    }
    bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {
        
        Vector ray_origin_vector = ray.origin - Point(0.0f);
        Vector ray_direction = ray.direction;
        // Check if the ray hits the sphere
        // based on "Mitternachtsformel"
        // if the following term is smaller than 0 that would correspond
        // to sqrt of a negative number which means no intersection
        float hit_value = (2 * ray_origin_vector.dot(ray_direction)) *
        (2 * ray_origin_vector.dot(ray_direction)) - 
        4 * ray_direction.dot(ray_direction) * 
        (ray_origin_vector.dot(ray_origin_vector) - 1);
        // check here
        if (hit_value < 0) {
            return false;
        }


        float t1;
        float t2;
        if (hit_value == 0) {
            t1 = - 1 * ray_origin_vector.dot(ray_direction) / 
            ray_direction.dot(ray_direction);
            its.t = t1;
        } else {
            t1 = (- 2 * ray_origin_vector.dot(ray_direction) + hit_value) / 
            ray_direction.dot(ray_direction);
            t2 = (- 2 * ray_origin_vector.dot(ray_direction) - hit_value) / 
            ray_direction.dot(ray_direction);
            
            // if both t are negative return false
            // we look for the smallest t which is greater than 0
            if (min(t1, t2) >= 0) {
                its.t = min(t1, t2);
            } else if (max(t1, t2) >= 0) {
                its.t = max(t1, t2);
            } else {
                return false;
            }
        }

        // position is the hit point
        its.position = ray(its.t);

        // calculate the normal vector of the hit point
        its.frame = Frame((ray(its.t) - Point(0)).normalized());

                
        its.wo = - ray_direction;


        return true;
    }
    Bounds getBoundingBox() const override {
        return Bounds(Point{-1, -1, -1}, Point{1, 1, 1});
    }
    Point getCentroid() const override {
        return Point{0, 0, 0};
    }
    AreaSample sampleArea(Sampler &rng) const override {
        NOT_IMPLEMENTED
    }
    std::string toString() const override {
    return "Sphere[]";
    }
};
}
REGISTER_SHAPE(Sphere, "sphere")