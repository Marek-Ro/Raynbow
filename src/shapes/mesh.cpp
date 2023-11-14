#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
    bool m_smoothNormals;

protected:
    int numberOfPrimitives() const override {
        return int(m_triangles.size());
    }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its, Sampler &rng) const override {
        // weights:
        // man kann ja mit zwei Vektoren einen dritten darstellen 
        // (vector a, vector b, vector zu Punkt c ist a + b)
        // wir haben nur genormte Vektoren a und b
        // diese multiplizieren wir mit dem weight w

        const float EPSILON = 0.0000001;

        Vector ray_origin_vector = ray.origin - Point(0.0f);
        Vector ray_direction = ray.direction;

        Vector3i vertices_indices = m_triangles[primitiveIndex];
        Vertex v0 = m_vertices[vertices_indices.x()];
        Vertex v1 = m_vertices[vertices_indices.y()];
        Vertex v2 = m_vertices[vertices_indices.z()];

        Vector v0v1 = v1.position - v0.position;
        Vector v0v2 = v2.position - v0.position;

        Vector pvec = ray_direction.cross(v0v2);
        float determinant = v0v1.dot(pvec);

        // if the determinant is negative, the triangle is 'back facing.'
        // if the determinant is close to 0, the ray misses the triangle
        // ray and triangle are parallel if det is close to 0
        if (determinant < EPSILON) {
            return false;
        } else if (fabs(determinant) < EPSILON) {               // fabs is float absolute value I guess
            return false;
        }

        float invDet = 1 / determinant;

        Vector tvec = ray_origin_vector - v0.position;
        float u = tvec.dot(pvec) * invDet;
        if (u > 1 || u < 0) {
            return false;
        }

        Vector qvec = tvec.cross(v0v1);
        float v = ray_direction.dot(qvec) * invDet;
        if (v < 0 || u + v > 1) {
            return false;
        }


        float t_candidate = v0v2.dot(qvec) * invDet;


        if (its.t > t_candidate) {
            its.t = t_candidate;

            Point hit_point = ray(its.t);
            its.position = hit_point;

            // calculate the normal vector of the hit point
            its.frame = Frame((hit_point - Point(0)).normalized());
            its.wo = -ray_direction;
            return true;
        } else {
            return false;
        }


        // hints:
        // * use m_triangles[primitiveIndex] to get the vertex indices of the triangle that should be intersected
        // * if m_smoothNormals is true, interpolate the vertex normals from m_vertices
        //   * make sure that your shading frame stays orthonormal!
        // * if m_smoothNormals is false, use the geometrical normal (can be computed from the vertex positions)
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        Vector3i vertices_indices = m_triangles[primitiveIndex];
        Vertex p1 = m_vertices[vertices_indices.x()];
        Vertex p2 = m_vertices[vertices_indices.y()];
        Vertex p3 = m_vertices[vertices_indices.z()];

        float x1 = p1.position.x();
        float x2 = p2.position.x();
        float x3 = p3.position.x();
        float min_x = min(x1, min(x2, x3));
        float max_x = max(x1, max(x2, x3));

        float y1 = p1.position.y();
        float y2 = p2.position.y();
        float y3 = p3.position.y();
        float min_y = min(y1, min(y2, y3));
        float max_y = max(y1, max(y2, y3));

        float z1 = p1.position.z();
        float z2 = p2.position.z();
        float z3 = p3.position.z();
        float min_z = min(z1, min(z2, z3));
        float max_z = max(z1, max(z2, z3));

        return Bounds(Point{min_x, min_y, min_z}, Point{max_x, max_y, max_z});
    }

    Point getCentroid(int primitiveIndex) const override {
        // (A_x + B_x + C_x) / 3, (A_y + B_y + C_y) / 3 ...
        Vertex A = m_vertices[m_triangles[primitiveIndex].x()];
        Vertex B = m_vertices[m_triangles[primitiveIndex].y()];
        Vertex C = m_vertices[m_triangles[primitiveIndex].z()];
        Point centroid;
        centroid.x() = (A.position.x() + B.position.x() + C.position.x()) / 3;
        centroid.y() = (A.position.y() + B.position.y() + C.position.y()) / 3;
        centroid.z() = (A.position.z() + B.position.z() + C.position.z()) / 3;
        return centroid;
    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath.string(), m_triangles, m_vertices);
        logger(EInfo, "loaded ply with %d triangles, %d vertices",
            m_triangles.size(),
            m_vertices.size()
        );
        buildAccelerationStructure();
    }

    AreaSample sampleArea(Sampler &rng) const override {
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
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

REGISTER_SHAPE(TriangleMesh, "mesh")
