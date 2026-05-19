#include <vulkan_raii.hpp>
#include <glm.hpp>

#include "resourceManager.h"


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



class Mesh : public Resource {
private:
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::DeviceSize vertexBufferOffset;
    uint32_t vertexCount = 0;

    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;
    vk::DeviceSize indexBufferOffset;
    uint32_t indexCount = 0;

public:
    explicit Mesh(const std::string& id) : Resource(id) {}

    ~Mesh() override {
        Unload();
    }

private:
    bool doLoad();
    bool doUnload();

    vk::Buffer GetVertexBuffer() const { return vertexBuffer; }
    vk::Buffer GetIndexBuffer() const { return indexBuffer; }
    uint32_t GetVertexCount() const { return vertexCount; }
    uint32_t GetIndexCount() const { return indexCount; }
};