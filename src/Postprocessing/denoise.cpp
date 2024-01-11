// While the denoiser might only require your rendered image as input, you will also want to feed images of normals, albedo
#include <iostream>
#include <vector>
#include <tinyexr.h>

#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>
namespace lightwave {


class denoise : public Postprocess {
public:
    bool hasNormals;
    bool hasAlbedo;
    ref<Image> m_albedo;
    ref<Image> m_normals;


    denoise(const Properties &properties)
        : Postprocess(properties) {
        m_albedo = properties.get<Image>("albedo");
        m_normals = properties.get<Image>("normals");
            
    }

    void execute() override {
        float width, height;
        width = m_input->resolution().x();
        height = m_input->resolution().y();

        m_output->initialize(m_input->resolution());

        // Create an Open Image Denoise device
        oidn::DeviceRef device = oidn::newDevice(oidn::DeviceType::CPU);
        device.commit();

        // Create a filter for denoising a beauty (color) image using optional auxiliary images too
        // This can be an expensive operation, so try no to create a new filter for every image!
        oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
        filter.set("hdr", true); // beauty image is HDR

        filter.setImage("color", m_input->data(),  oidn::Format::Float3, width, height); // beauty
        filter.setImage("albedo", m_albedo->data(), oidn::Format::Float3, width, height); // auxiliary
        filter.setImage("normal", m_normals->data(), oidn::Format::Float3, width, height); // auxiliary
        filter.setImage("output", m_output->data(),  oidn::Format::Float3, width, height); // denoised beauty
        filter.commit();

        // Filter the beauty image
        filter.execute();

        m_output->save();

        // Check for errors
        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
            std::cout << "Error: " << errorMessage << std::endl;
    }
    std::string toString() const override {
        return "denoise";
    }

};

} // namespace lightwave
REGISTER_POST(denoise, "denoising")