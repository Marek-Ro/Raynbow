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
        if (m_border == BorderMode::Clamp) {
            point = border_mode_clamp(uv);
        }
        else {
            point = border_mode_repeat(uv);
        }

        // scale the point according to the image resolution
        point.x() = point.x() * (m_image->resolution().x()-1);
        point.y() = point.y() * (m_image->resolution().y()-1);

        Point2i pixel;

        if (m_filter == FilterMode::Nearest) {
            pixel = filter_mode_nearest(point);
        }
        else {
            return filter_mode_bilinear(point);
        }

        // making sure pixel is in image
        pixel.x() = pixel.x() % m_image->resolution().x();
        pixel.y() = pixel.y() % m_image->resolution().y();

        return m_image->operator()(pixel);
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
private:
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

    Color filter_mode_bilinear(const Point2 point) const {
        Point2i Q11, Q12, Q21, Q22;
        Q11.x() = floor(point.x());
        Q11.y() = floor(point.y());
        Q12.x() = floor(point.x());
        Q12.y() = floor(point.y()+1.0f);
        Q21.x() = floor(point.x()+1.0f);
        Q21.y() = floor(point.y());
        Q22.x() = floor(point.x()+1.0f);
        Q22.y() = floor(point.y()+1.0f);
        
        //x1
        float f = Q11.x() - point.x();
        Color a = m_image->operator()(Q11);
        Color b = m_image->operator()(Q21);
        Color c1 = lin_interpolate(a,b,f);
        
        //x2
        f = Q12.x() - point.x();
        a = m_image->operator()(Q12);
        b = m_image->operator()(Q22);
        Color c2 = lin_interpolate(a,b,f);
        
        // y1
        f = Q11.y() - point.y();
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
