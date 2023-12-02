#include <lightwave.hpp>

namespace lightwave
{

    class ImageTexture : public Texture
    {
        enum class BorderMode
        {
            Clamp,
            Repeat,
        };

        enum class FilterMode
        {
            Nearest,
            Bilinear,
        };

        ref<Image> m_image;
        float m_exposure;
        BorderMode m_border;
        FilterMode m_filter;

    public:
        ImageTexture(const Properties &properties)
        {
            if (properties.has("filename"))
            {
                m_image = std::make_shared<Image>(properties);
            }
            else
            {
                m_image = properties.getChild<Image>();
            }
            m_exposure = properties.get<float>("exposure", 1);

            m_border =
                properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                               {
                                                   {"clamp", BorderMode::Clamp},
                                                   {"repeat", BorderMode::Repeat},
                                               });

            m_filter = properties.getEnum<FilterMode>(
                "filter", FilterMode::Bilinear,
                {
                    {"nearest", FilterMode::Nearest},
                    {"bilinear", FilterMode::Bilinear},
                });
        }

        Color evaluate(const Point2 &uv) const override
        {
            Point2 point;
            Point2 uv2 = uv;

            // invert the y-axis to so that (0,0) is in the bottom left corner
            uv2.y() = 1 - uv2.y();

            int width = m_image->resolution().x();
            int height = m_image->resolution().y();

            if (m_filter == FilterMode::Bilinear)
            {

                // scale the point according to the image resolution
                point.x() = uv2.x() * width - 0.5f;
                point.y() = uv2.y() * height - 0.5f;

                Point2 p_rounded_down = Point2(std::floor(point.x()), std::floor(point.y()));

                Point2 position_im_pixel = point - p_rounded_down;

                // the four pixels around our point
                Point2i p11 = Point2i((int)p_rounded_down.x(), (int)p_rounded_down.y());
                Point2i p12 = Point2i((int)p_rounded_down.x(), (int)p_rounded_down.y() + 1);
                Point2i p21 = Point2i((int)p_rounded_down.x() + 1, (int)p_rounded_down.y());
                Point2i p22 = Point2i((int)p_rounded_down.x() + 1, (int)p_rounded_down.y() + 1);

                // Border Handling
                // apply to our four surrounding pixels, which we later use in our interpolate function
                if (m_border == BorderMode::Clamp)
                {
                    // Clamp
                    p11 = clamp_point(p11, width, height);
                    p12 = clamp_point(p12, width, height);
                    p21 = clamp_point(p21, width, height);
                    p22 = clamp_point(p22, width, height);
                }
                else
                {
                    // Repeat
                    p11 = repeat_point(p11, width, height);
                    p12 = repeat_point(p12, width, height);
                    p21 = repeat_point(p21, width, height);
                    p22 = repeat_point(p22, width, height);
                }

                // Bilinear Interpolation
                // to linear interpolation two times in x-direction and one time in y-direction

                // x1
                Color a = m_image->operator()(p11);
                Color b = m_image->operator()(p21);
                Color c1 = lin_interpolate(a, b, position_im_pixel.x());

                // x2
                a = m_image->operator()(p12);
                b = m_image->operator()(p22);
                Color c2 = lin_interpolate(a, b, position_im_pixel.x());

                // y1
                Color c3 = lin_interpolate(c1, c2, position_im_pixel.y());

                return m_exposure * c3;
            }
            // filter mode nearest
            else
            {
                if (m_border == BorderMode::Repeat)
                {
                    // take mod 1 to repeat
                    uv2.x() = uv2.x() < 0 ? 1 - fmod(abs(uv2.x()), 1.0f) : fmod(abs(uv2.x()), 1.0f);
                    uv2.y() = uv2.y() < 0 ? 1 - fmod(abs(uv2.y()), 1.0f) : fmod(abs(uv2.y()), 1.0f);
                }
                return m_exposure * m_image->operator()(uv2);
            }
        }

        std::string toString() const override
        {
            return tfm::format("ImageTexture[\n"
                               "  image = %s,\n"
                               "  exposure = %f,\n"
                               "]",
                               indent(m_image), m_exposure);
        }

        // Helper functions
    private:
        Point2i clamp_point(Point2i p, int width, int height) const
        {
            // clamps our point between [0;width] and [0;height]
            return Point2i(max(0, min(p.x(), width - 1)), max(0, min(p.y(), height - 1)));
        }

        Point2i repeat_point(Point2i p, int width, int height) const
        {
            // take the modulo to repeat the point from: [0;width] and [0;height]
            p.x() = p.x() % width;
            p.y() = p.y() % height;
            // make sure it is positive, so no out of bounds
            p.x() = p.x() < 0 ? p.x() + width : p.x();
            p.y() = p.y() < 0 ? p.y() + height : p.y();
            return p;
        }
        Color lin_interpolate(Color a, Color b, float f) const
        {
            // f = distance between point and one of the colors
            // return the combined color of a and b depending on the distance our point
            return (a * (1.0 - f)) + (b * f);
        }
    };

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
