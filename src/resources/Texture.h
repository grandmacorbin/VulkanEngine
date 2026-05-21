

#include "resourceManager.h"
#include "vulkan_raii.hpp"



class Texture : public Resource {
private:
    vk::Image image;
    vk::DeviceMemory memory;
    vk::DeviceSize offset;
    vk::ImageView imageView;
    vk::Sampler sampler;

    int width = 0;
    int height = 0;
    int channels = 0;

public:
    explicit Texture(const std::string& id) : Resource(id) {}

    ~Texture() override {
        Unload();
    }

    vk::Image GetImage() const { return image; }
    vk::ImageView GetImageView() const { return imageView; }
    vk::Sampler GetSampler() const { return sampler; }

protected:
    bool doLoad() override;
    bool doUnload() override;

private:
    unsigned char* LoadImageData(const std::string& filepath, int* width, int* height, int* channles);
    void FreeImageData(unsigned char* data);
    void CreateVulkanImage(unsigned char* data, int width, int height, int channels);
    vk::Device GetDevice();
};