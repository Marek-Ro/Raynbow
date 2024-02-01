#include <lightwave.hpp>
namespace lightwave
{
    class Fog : public Shape
    {
    public:
        Fog(const Properties &properties)
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
            populate(its, Point(1000, 1000, 1000));
            return true;
        }
        Bounds getBoundingBox() const override
        {
            return Bounds(Point{-1000, -1000, -1000}, Point{1000, 1000, 1000});
        }
        Point getCentroid() const override
        {
            return Point{0, 0, 0};
        }
        AreaSample sampleArea(Sampler &rng) const override{
            return AreaSample::invalid();
        }
        std::string toString() const override {
            return "Fog[]";
        }
    };
}
REGISTER_SHAPE(Fog, "fog")