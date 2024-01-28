#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class Volume : public Shape {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;

    Vector m_centroid;

    float m_x_length;
    float m_y_length;
    float m_z_length;

    Vector p1;
    Vector p2;
    Vector p3;
    Vector p4;

    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    

protected:

    bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {

        float t = intersectAABB(getBoundingBox(), ray);
        
        if (t == Infinity) return false;

        its.t = t;
        its.position = ray(its.t);
        its.frame = Frame(ray.direction);
        its.volume_intersection = true;

        assert(t > 0);

        return true;
    }

    // from accel.hpp

    /// @brief Performs a slab test to intersect a bounding box with a ray,
    /// returning Infinity in case the ray misses.
    float intersectAABB(const Bounds &bounds, const Ray &ray) const
    {
        float Epsilon = 1e-8f;
        // but this only saves us ~1%, so let's not do it. intersect all axes at
        // once with the minimum slabs of the bounding box
        const auto t1 = (bounds.min() - ray.origin) / ray.direction;
        // intersect all axes at once with the maximum slabs of the bounding box
        const auto t2 = (bounds.max() - ray.origin) / ray.direction;

        // the elementwiseMin picks the near slab for each axis, of which we
        // then take the maximum
        const auto tNear = elementwiseMin(t1, t2).maxComponent();
        // the elementwiseMax picks the far slab for each axis, of which we then
        // take the minimum
        const auto tFar = elementwiseMax(t1, t2).minComponent();
        if (tFar < tNear)
            return Infinity; // the ray does not intersect the bounding box
        if (tFar < Epsilon)
            return Infinity; // the bounding box lies behind the ray origin

//        logger(EInfo, "tNear %f tFar%f", tNear, tFar);

        // for volumes they cant be negative
        if (tNear > Epsilon) return tNear;

        return tFar;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point{m_centroid.x() + m_x_length, m_centroid.y() + m_y_length, m_centroid.z() + m_z_length}, 
        Point{m_centroid.x() - m_x_length, m_centroid.y() - m_y_length, m_centroid.z() - m_z_length});
    }

    Point getCentroid() const override {
        return m_centroid;
    }

public:
    Volume(const Properties &properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename", "");
        m_centroid = properties.get<Vector>("centroid", Vector(0));
        m_x_length = properties.get<float>("x", 1);
        m_y_length = properties.get<float>("y", 1);
        m_z_length = properties.get<float>("z", 1);

        if (m_originalPath != "") {
            readPLY(m_originalPath.string(), m_triangles, m_vertices);
        }
        
        logger(EInfo, "loaded ply with %d triangles, %d vertices",
            m_triangles.size(),
            m_vertices.size()
        );
    }

    AreaSample sampleArea(Sampler &rng) const override {
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Volume[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string()
        );
    }
};

}

REGISTER_SHAPE(Volume, "volume")
