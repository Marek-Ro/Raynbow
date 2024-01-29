#include <lightwave.hpp>
namespace lightwave
{
    class Sphere : public Shape
    {
    public:
        Sphere(const Properties &properties)
        {
        }
        /**
     * @brief Constructs a surface event for a given position, used by @ref intersect to populate the @ref Intersection
     * and by @ref sampleArea to populate the @ref AreaSample .
     * @param surf The surface event to populate with texture coordinates, shading frame and area pdf
     * @param position The hitpoint (i.e., point in [-1,-1,0] to [+1,+1,0]), found via intersection or area sampling
     */
    inline void populate(SurfaceEvent &surf, const Point &position) const {
        surf.position = position;
        float u = 0.5 + (atan2(position.x(), position.z()) / (2 * Pi));
        float v = 0.5 - (asin(position.y()) / Pi);
        surf.uv = Point2(u, v);
        
        surf.frame = Frame((position - Point(0)).normalized());
        /*// the tangent always points in positive x direction
        surf.frame.tangent = Vector(1, 0, 0);
        // the bitagent always points in positive y direction
        surf.frame.bitangent = Vector(0, 1, 0);
        // and accordingly, the normal always points in the positive z direction
        surf.frame.normal = Vector(0, 0, 1); */

        // since we sample the area uniformly, the pdf is given by 1/surfaceArea
        surf.pdf = Inv4Pi;
    }
        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override
        {
            // define a new epsilon to pass furnace
            float Epsilon = 1e-4f; 

            Vector ray_origin_vector = ray.origin - Point(0.0f);
            Vector ray_direction = ray.direction;
            /*
            Check if the ray hits the sphere
            based on "Mitternachtsformel"
            if the following term is smaller than 0 that would correspond
            to sqrt of a negative number which means no intersection
            */
            float hit_value = (2 * ray_origin_vector.dot(ray_direction)) *
                                  (2 * ray_origin_vector.dot(ray_direction)) -
                              4 * ray_direction.dot(ray_direction) *
                                  (ray_origin_vector.dot(ray_origin_vector) - 1);

            // check here if intersection happens at all
            if (hit_value < 0)
            {
                return false;
            }

            // 2 possible intersection points
            float t1, t2;
            // t_candidate is the earliest intersection point we find
            float t_candidate;
            // in case of hit value == 0 there is only one
            if (hit_value < Epsilon && hit_value > -Epsilon)
            {
                t_candidate = -1 * ray_origin_vector.dot(ray_direction) /
                              ray_direction.dot(ray_direction);
            }
            else
            {
                t1 = ((-2 * ray_origin_vector.dot(ray_direction)) + sqrt(hit_value)) /
                     2 * ray_direction.dot(ray_direction);
                t2 = ((-2 * ray_origin_vector.dot(ray_direction)) - sqrt(hit_value)) /
                     2 * ray_direction.dot(ray_direction);

                // if both t are negative return false
                // we look for the smallest t which is greater than 0
                if (min(t1, t2) >= Epsilon)
                {
                    t_candidate = min(t1, t2);
                }
                else if (max(t1, t2) >= Epsilon)
                {
                    t_candidate = max(t1, t2);
                }
                else
                {
                    // in this case both t1 and t2 are < 0 so we have no t_candidate
                    return false;
                }
            }

            // compare our intersection point to the closest intersection so far
            if (t_candidate > its.t || (t_candidate < Epsilon && t_candidate > -Epsilon))
            {
                return false;
            }

            Vector position_hit_point = ((Vector)(ray(t_candidate))).normalized();

            // compute uv
            float u = 0.5 + (atan2(position_hit_point.x(), position_hit_point.z()) / (2 * Pi));
            float v = 0.5 - (asin(position_hit_point.y()) / Pi);
            Point2 uv = Point2(u, v);
            if (its.alpha_mask != nullptr) {
                // valid alpha_mask value
                // check if the intersection still occurs
                if (its.alpha_mask->evaluate(uv).r() < rng.next()) {
                    return false;
                }
            }
            its.t = t_candidate;
            populate(its,position_hit_point);
            // calculate the normal vector of the hit point
            
            return true;
        }
        Bounds getBoundingBox() const override
        {
            return Bounds(Point{-1, -1, -1}, Point{1, 1, 1});
        }
        Point getCentroid() const override
        {
            return Point{0, 0, 0};
        }
        AreaSample sampleArea(Sampler &rng) const override{
            Point2 rnd = rng.next2D(); // sample a random point in [0,0]..[1,1]
            Point position { squareToUniformSphere(rnd)}; // stretch the random point to [-1,-1]..[+1,+1] and set z=0

            AreaSample sample;
            populate(sample, position); // compute the shading frame, texture coordinates and area pdf (same as intersection)
            return sample;
        }
        std::string toString() const override {
            return "Sphere[]";
        }
    };
}
REGISTER_SHAPE(Sphere, "sphere")