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


        Point2i pixel;

        if (m_filter == FilterMode::Nearest) {
            pixel = filter_mode_nearest(point);
        }
        else {
            pixel = filter_mode_bilinear(point);
        }
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
        1.000...1 -> 0
        1.5 -> 0.5
        2 -> 1

        for negativ numbers this is just mod 1

        for positiv numbers:
        mod 1 works almost everywhere
        but not for 2 -> 1
        hence use if(i > 0 && i mod 1 == 0)
        BUT this will result in problems with floating point numbers
        
        */
        //float u = fmod(uv.x(), 1.0); 
        //float v = fmod(uv.y(), 1.0);
        float u = 1.0;
        float v = 1.0;
        return Point2(u,v);
    }

    Point2i filter_mode_nearest(const Point2 point) const {
        // TODO
        return Point2i(0,0);
    }

    Point2i filter_mode_bilinear(const Point2 point) const {
        // TODO
        return Point2i(0,0);
    }

};



} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
