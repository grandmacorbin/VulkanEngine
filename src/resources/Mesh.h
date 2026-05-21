#include <array>

#include "vulkan_raii.hpp"
#include "glm.hpp"
#include "resourceManager.h"


class Mesh : public Resource {
private:
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::DeviceSize vertexBufferOffset;
    uint32_t vertexCount = 0;

    vk::raii::Buffer indexBuffer = nullptr;
    vk::raii::DeviceMemory indexBufferMemory = nullptr;
    vk::DeviceSize indexBufferOffset;
    uint32_t indexCount = 0;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 texCoord;

        // need vertexInputBindings
        static vk::VertexInputBindingDescription getBindingDescription()
        {
            vk::VertexInputBindingDescription VertexBindingDescription;
            VertexBindingDescription.binding = 0;
            VertexBindingDescription.stride = sizeof(Vertex);
            VertexBindingDescription.inputRate = vk::VertexInputRate::eVertex;
            return VertexBindingDescription;
        }
        // need vertexInputAttributeDescriptions
        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 3> AttributeDescriptions;
            AttributeDescriptions[0].location = 0;
            AttributeDescriptions[0].binding = 0;
            AttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            AttributeDescriptions[0].offset = offsetof(Vertex, position);
            AttributeDescriptions[1].location = 1;
            AttributeDescriptions[1].binding = 0;
            AttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            AttributeDescriptions[1].offset = offsetof(Vertex, color);
            AttributeDescriptions[2].location = 2;
            AttributeDescriptions[2].binding = 0;
            AttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
            AttributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return AttributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && texCoord == other.texCoord;
        }
    };

public:
    explicit Mesh(const std::string& id) : Resource(id) {}

    ~Mesh() override {
        Unload();
    }

private:
    bool doLoad();
    bool doUnload();

    bool LoadMeshData(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    void CreateVertexBuffer(const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(const std::vector<uint32_t>& indices);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    vk::raii::Device GetDevice();

    const vk::raii::Buffer& GetVertexBuffer() const { return vertexBuffer; }
    const vk::raii::Buffer& GetIndexBuffer() const { return indexBuffer; }
    uint32_t GetVertexCount() const { return vertexCount; }
    uint32_t GetIndexCount() const { return indexCount; }
};