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
    denoise(const Properties &properties)
        : Postprocess(properties) {
            m_output->initialize(m_input->resolution());
            hasNormals = false;
            hasAlbedo = false;
    }
/*
    void loadImage(const std::string& filename, float*& buffer, int width, int height) {
        float* image = nullptr;

        const char* err;
        int ret = LoadEXR(&image, &width, &height, filename.c_str(), &err);

        if (ret != TINYEXR_SUCCESS) {
            std::cerr << "Error loading image: " << err << std::endl;
            FreeEXRErrorMessage(err); // Free the error message allocated by tinyexr
            // Handle error
            return;
        }

        // Assuming the loaded image is in RGBA format, convert it to RGB
        buffer = new float[width * height * 3];
        for (int i = 0; i < width * height; ++i) {
            buffer[i * 3 + 0] = image[i * 4 + 0];
            buffer[i * 3 + 1] = image[i * 4 + 1];
            buffer[i * 3 + 2] = image[i * 4 + 2];
        }

        // Free the memory allocated by tinyexr
        free(image);
    }

    void Image::loadImage(const std::filesystem::path &path, bool isLinearSpace) {


        const auto extension = path.extension();
        logger(EInfo, "loading image %s", path);
        if (extension == ".exr") {
            // loading of EXR files is handled by TinyEXR
            float *data;
            const char *err;
            if (LoadEXR(&data, &m_resolution.x(), &m_resolution.y(),
                        path.generic_string().c_str(), &err)) {
                lightwave_throw("could not load image %s: %s", path, err);
            }

            m_data.resize(m_resolution.x() * m_resolution.y());
            auto it = data;
            for (auto &pixel : m_data) {
                for (int i = 0; i < pixel.NumComponents; i++)
                    pixel[i] = *it++;
                it++; // skip alpha channel
            }
            free(data);
            } else {
                lightwave_throw("denoiser can only load .exr files");
            }
    }

*/


    void loadImageDataIntoBuffer(oidn::BufferRef& pBuffer, Image& image) {
        
//        int width = image.resolution().x();
//        int height = image.resolution().y();
//        Color* my_data = image.data();

        // Convert the data to float array
//        for (int i = 0; i < width * height * 3; ++i) {
//            if (i % 3 == 0) { pBuffer.getData()[i] = my_data[i].r(); }
//            if (i % 3 == 1) { pBuffer.getData()[i] = my_data[i].g(); }
//            if (i % 3 == 2) { pBuffer.getData()[i] = my_data[i].b(); }
//        }

        // Assuming lightwave::Image provides .width(), .height(), and .data() functions
        const size_t width = image.resolution().x();
        const size_t height = image.resolution().y();
        const Color* imageData = image.data();

        // Assuming oidn::BufferRef provides .write function
        const size_t pixelSize = sizeof(Color); // Replace Color with your actual pixel type
        const size_t byteSize = width * height * pixelSize;

        // Assuming oidnBuffer is your OIDN buffer
        pBuffer.write(0, byteSize, imageData);

        logger(EInfo, "copied image %s to oidn buffer", image.toString());
    }

    void execute() override {
        float width, height;
        width = m_input->resolution().x();
        height = m_input->resolution().y();

        // Create an Open Image Denoise device
        oidn::DeviceRef device = oidn::newDevice(oidn::DeviceType::CPU);
        device.commit();

        // Create buffers for input/output images accessible by both host (CPU) and device (CPU/GPU)
        oidn::BufferRef colorBuf  = device.newBuffer(width * height * 3 * sizeof(float));
        //oidn::BufferRef albedoBuf = ...
//        oidn::BufferRef albedoBuf = device.newBuffer(width * height * 3 * sizeof(float));
        oidn::BufferRef normalBuf = device.newBuffer(width * height * 3 * sizeof(float));

        // loads the images
        Image noisyImage = Image("./noisy.exr", true);
        logger(EInfo, "this is our noisy image %s", noisyImage.toString());

//        Image albedoImage = Image("./albedo.exr", true);
//        Image normalImage = Image("./normals.exr", true);

        // fill buffers
        loadImageDataIntoBuffer(colorBuf, noisyImage);
//        loadImageDataIntoBuffer(albedoBuf, albedoImage);
//        loadImageDataIntoBuffer(normalBuf, normalImage);

        // Create a filter for denoising a beauty (color) image using optional auxiliary images too
        // This can be an expensive operation, so try no to create a new filter for every image!
        oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
        filter.setImage("color",  colorBuf,  oidn::Format::Float3, width, height); // beauty
//        filter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height); // auxiliary
//        filter.setImage("normal", normalBuf, oidn::Format::Float3, width, height); // auxiliary
        filter.setImage("output", colorBuf,  oidn::Format::Float3, width, height); // denoised beauty
        filter.set("hdr", true); // beauty image is HDR
        filter.commit();

        // Filter the beauty image
        filter.execute();

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