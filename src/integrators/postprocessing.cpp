// While the denoiser might only require your rendered image as input, you will also want to feed images of normals, albedo

#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>
namespace lightwave {


class denoise : public Postprocess {
    oidn::DeviceRef device = oidn::newDevice();
    //device.commit();
    // does not work?
    // https://www.openimagedenoise.org/documentation.html

};

} // namespace lightwave