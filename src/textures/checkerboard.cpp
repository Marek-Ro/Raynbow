#include <lightwave.hpp>

namespace lightwave {

class CheckerboardTexture : public Texture {
    Color color0;
    Color color1;
    Vector2 scale;

public:
    CheckerboardTexture(const Properties &properties) {
        color0 = properties.get<Color>("color0", Color(0));
        color1 = properties.get<Color>("color1", Color(1));
        scale = properties.get<Vector2>("scale", Vector2(1.0f, 1.0f));
    }

    Color evaluate(const Point2 &uv) const override {
        
        Vector2 my_uv = uv - Point2(0);

        my_uv.x() = fabs(std::floor(my_uv.x() * scale.x()));
        my_uv.y() = fabs(std::floor(my_uv.y() * scale.y()));

    
        Color result = ((int)my_uv.x() % 2) + ((int)my_uv.y() % 2) != 1 ? color0 : color1;

        return result;
    }

    std::string toString() const override {
        return tfm::format("CheckerboardsTexture[\n"
                           "  value = %s\n"
                           "]",
                           indent(color0));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
