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
            pixel = filter_mode_bilinear(point);
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
    }

    Point2 border_mode_repeat(const Point2 &uv) const {
        // TODO

        /* Idea
        We need a function that does the following
        -1 -> 0
        -0.5 -> 0.5 
        -0.999...9 -> 1
        0 -> 0
        0.5 -> 0.5
        1 -> 1
        1.000...1 -> 0.00000001
        1.5 -> 0.5

        1.99999999 -> 0.99999999 = 1
        2 -> 1
        2 - 1
        2.00000000000001 -> 0.0000000001 = 0
        10,9 - 10
        for negativ numbers this is just mod 1

        for positiv numbers:
        mod 1 works almost everywhere
        but not for 2 -> 1
        hence use if(i > 0 && i mod 1 == 0)
        BUT this will result in problems with floating point numbers
        
        */
//        float u = fmod(uv.x(), 1.0); 
//        float v = fmod(uv.y(), 1.0);

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

    Point2i filter_mode_bilinear(const Point2 point) const {
        // from: https://en.wikipedia.org/wiki/Bilinear_interpolation
        //
        // we got a point that lies between four other pixels
        // round x and y down and up 
        // Example:
        // (1.2, 2,9) -> Q11(1,2), Q12(1,3), Q21(2,2), Q22(2,3)
        //            -> Q11(x1,y1), Q12(x1,y2), Q21(x2,y1), Q22(x2,y2)
        
        // TODO evt. direkt zu point2i machen weil rounding error später
        Point2 Q11, Q12, Q21, Q22  = point;
        Q11.x() = floor(Q11.x());
        Q11.y() = floor(Q11.y());

        Q12.x() = floor(Q12.x());
        Q12.y() = floor(Q12.y()+1.0f);

        Q21.x() = floor(Q21.x()+1.0f);
        Q21.y() = floor(Q21.y());

        Q22.x() = floor(Q22.x()+1.0f);
        Q22.y() = floor(Q22.y()+1.0f);


        // linear interpolation in the x-Direction
        Point2 FX1 = ((Q21.x() - point.x())/(Q21.x() - Q11.x()))* (Q11-Point2(0)) + ((point.x() - Q11.x())/(Q21.x() - Q11.x()))* (Q21-Point2(0));
        Point2 FX2 = ((Q21.x() - point.x())/(Q21.x() - Q11.x()))* (Q12-Point2(0)) + ((point.x() - Q11.x())/(Q21.x() - Q11.x()))* (Q22-Point2(0));
        
        // linear interpolation in the y-Direction
        Point2 FXY = ((Q22.y() - point.y())/(Q22.y() - Q11.y()))* (FX1-Point2(0)) + ((point.y() - Q11.y())/(Q22.y() - Q11.y()))* (FX2-Point2(0));


        Point2i pixel = Point2i(FXY.x(), FXY.y());
        //return filter_mode_nearest(point);
        return pixel;
    }

};



} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
