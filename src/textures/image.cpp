#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                           {
                                               { "clamp", BorderMode::Clamp },
                                               { "repeat", BorderMode::Repeat },
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter", FilterMode::Bilinear,
            {
                { "nearest", FilterMode::Nearest },
                { "bilinear", FilterMode::Bilinear },
            });
    }

    Color evaluate(const Point2 &uv) const override {
        Point2 point;
        Point2 uv2 = uv;
        uv2.y() = 1 - uv2.y();

        int width = m_image->resolution().x();
        int height = m_image->resolution().y();

        if (m_filter == FilterMode::Bilinear) {

            // scale the point according to the image resolution
            point.x() = uv2.x() * width - 0.5f;
            point.y() = uv2.y() * height - 0.5f;

            Point2 p_abgerundet = Point2(std::floor(point.x()), std::floor(point.y()));

            Point2 position_im_pixel = point - p_abgerundet;

            Point2i p11 = Point2i((int)p_abgerundet.x(), (int)p_abgerundet.y());
            Point2i p12 = Point2i((int)p_abgerundet.x(), (int)p_abgerundet.y() + 1);
            Point2i p21 = Point2i((int)p_abgerundet.x() + 1, (int)p_abgerundet.y());
            Point2i p22 = Point2i((int)p_abgerundet.x() + 1, (int)p_abgerundet.y() + 1);

            // Border Handling

            if (m_border == BorderMode::Clamp) {
                // Clamp
                p11 = clamp_point(p11, width, height);
                p12 = clamp_point(p12, width, height);
                p21 = clamp_point(p21, width, height);
                p22 = clamp_point(p22, width, height);

            } else {
                // Repeat
                p11 = repeat_point(p11, width, height);
                p12 = repeat_point(p12, width, height);
                p21 = repeat_point(p21, width, height);
                p22 = repeat_point(p22, width, height);

            }

            // Bilinear Interpolation

            //x1
            Color a = m_image->operator()(p11);
            Color b = m_image->operator()(p21);
            Color c1 = lin_interpolate(a, b, position_im_pixel.x());

            //x2
            a = m_image->operator()(p12);
            b = m_image->operator()(p22);
            Color c2 = lin_interpolate(a, b, position_im_pixel.x());

            // y1
            Color c3 = lin_interpolate(c1, c2, position_im_pixel.y());

            return m_exposure * c3;

        } else {

            if (m_border == BorderMode::Repeat) {
                    uv2.x() = uv2.x() < 0 ? 1 - fmod(abs(uv2.x()), 1.0f): fmod(abs(uv2.x()), 1.0f);
                    uv2.y() = uv2.y() < 0 ? 1 - fmod(abs(uv2.y()), 1.0f): fmod(abs(uv2.y()), 1.0f);

                }

            // FilterMode Nearest
            return m_exposure * m_image->operator()(uv2);
        }
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
private:

    Point2i clamp_point(Point2i p, int width, int height) const {
        return Point2i(max(0, min(p.x(), width - 1)), max(0, min(p.y(), height - 1)));
    }

    Point2i repeat_point(Point2i p, int width, int height) const {
        p.x() = p.x() % width;
        p.y() = p.y() % height;
        // make sure it is positive, so no out of bounds
        p.x() = p.x() < 0 ? p.x() + width : p.x();
        p.y() = p.y() < 0 ? p.y() + height : p.y();
        return p;
    }

    Point2i find_closest_point(Point2 point, Point2i p11, Point2i p12, Point2i p21, Point2i p22, Point2 position_im_pixel) const {

        Point2 p11_f = Point2(p11.x(), p11.y());
        Point2 p12_f = Point2(p12.x(), p12.y());
        Point2 p21_f = Point2(p21.x(), p21.y());
        Point2 p22_f = Point2(p22.x(), p22.y());

        if ((p11_f - point).length() < (p12_f - point).length() &&
                    (p11_f - point).length() < (p21_f - point).length() &&
                    (p11_f - point).length() < (p22_f - point).length()) {
            return p11;
        }

        if ((p12_f - point).length() < (p11_f - point).length() &&
                    (p12_f - point).length() < (p21_f - point).length() &&
                    (p12_f - point).length() < (p22_f - point).length()) {
            return p12;
        }

        if ((p21_f - point).length() < (p11_f - point).length() &&
                    (p21_f - point).length() < (p12_f - point).length() &&
                    (p21_f - point).length() < (p22_f - point).length()) {
            return p21;
        }

        return p22;
    }

    // Maps from [-infinity, +infinity] to [0,1]
    Point2 border_mode_clamp(const Point2 &uv) const {
        float u = saturate(uv.x());
        float v = saturate(uv.y());
        return Point2(u,v);
    }

    float help_mod_1(float x) const {
        if (x >= 0 && x <= 1) {
            return x;
        } else if (x < 0) {
            return help_mod_1(x + 1);
        } else if (x > 1) {
            return help_mod_1(x - 1);
        }
        return 0;
    }

    Point2 border_mode_repeat(const Point2 &uv) const {
        float u = help_mod_1(uv.x()); 
        float v = help_mod_1(uv.y());

        return Point2(u,v);
    }

    Point2i filter_mode_nearest(const Point2 point) const {
        // round the floating point number to the nearest integer 
        int x = floor(point.x() + 0.5f);
        int y = floor(point.y() + 0.5f);
        Point2i pixel = Point2i(x,y);
        return pixel;
    }

    // ensures that the given pixel is inside the valid range
    Point2i pixel_in_range(Point2i pixel) const {
        if (pixel.x() > m_image->resolution().x()-1) {
            pixel.x() = m_image->resolution().x()-1;
        }
        if (pixel.y() > m_image->resolution().y()-1) {
            pixel.y() = m_image->resolution().y()-1;
        }
        if (pixel.x() < 0) {
            pixel.x() = 0;
        }
        if (pixel.y() < 0) {
            pixel.y() = 0;
        }

        return pixel;
    }

    // ensures that the given point is inside the valid range
    Point2 point_in_range(Point2 point) const {
        if (point.x() > m_image->resolution().x()-1) {
            point.x() = m_image->resolution().x()-1;
        }
        if (point.y() > m_image->resolution().y()-1) {
            point.y() = m_image->resolution().y()-1;
        }
        if (point.x() < 0) {
            point.x() = 0;
        }
        if (point.y() < 0) {
            point.y() = 0;
        }
        return point;
    }

    Color filter_mode_bilinear(const Point2 point2) const {
        Point2 point = point_in_range(point2);
        // create the 4 neighboring points
        Point2i Q11, Q12, Q21, Q22;
        Q11 = pixel_in_range(Point2i(floor(point.x()), floor(point.y())));
        Q12 = pixel_in_range(Point2i(floor(point.x()),floor(point.y()+1.0f)));
        Q21 = pixel_in_range(Point2i(floor(point.x()+1.0f),floor(point.y())));
        Q22 = pixel_in_range(Point2i(floor(point.x()+1.0f),floor(point.y()+1.0f)));
        
        
        /*Q11.x() = floor(point.x());
        Q11.y() = floor(point.y());

        Q12.x() = floor(point.x());
        Q12.y() = floor(point.y()+1.0f);

        Q21.x() = floor(point.x()+1.0f);
        Q21.y() = floor(point.y());
        Q22.x() = floor(point.x()+1.0f);
        Q22.y() = floor(point.y()+1.0f); */
        
        //x1
        float f = point.x()-Q11.x();
        Color a = m_image->operator()(Q11);
        Color b = m_image->operator()(Q21);
        Color c1 = lin_interpolate(a,b,f);
        
        //x2
        f = point.x()- Q12.x();
        a = m_image->operator()(Q12);
        b = m_image->operator()(Q22);
        Color c2 = lin_interpolate(a,b,f);
        
        // y1
        f = point.y()-Q11.y();
        Color c3 = lin_interpolate(c1,c2,f);


        Color color;
        return c3;
    }

    Color lin_interpolate(Color a, Color b, float f) const {
        return (a * (1.0 - f)) + (b * f);
    }

};



} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
