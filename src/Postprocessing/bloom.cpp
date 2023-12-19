#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>
namespace lightwave {


class denoise : public Postprocess {
public:
    denoise(const Properties &properties)
        : Postprocess(properties) {
    }
    void execute() override {
    }
};
} // namespace lightwave