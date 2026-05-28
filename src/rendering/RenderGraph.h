#include <string>
#include <functional>
#include <unordered_map>

#include "vulkan/vulkan_raii.hpp"

class RenderGraph {
private:
    struct Resource {
        std::string name;
        vk::Format format;
        vk::Extent2D extent;
        vk::ImageUsageFlags usage;
        vk::ImageLayout initialLayout;
        vk::ImageLayout finalLayout;

        vk::raii::Image image = nullptr;
        vk::raii::DeviceMemory memory = nullptr;
        vk::raii::ImageView view = nullptr;
    };

    struct Pass {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        std::function<void(vk::raii::CommandBuffer&)> executeFunc;
    };

    std::unordered_map<std::string, Resource> resources;
    std::vector<Pass> passes;
    std::vector<size_t> executionOrder;

    std::vector<vk::raii::Semaphore> semaphores;
    std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs;

    vk::raii::Device& device;

public:
    explicit RenderGraph(vk::raii::Device& dev) : device(dev) {}

    void AddResource(const std::string& name, vk::Format format, vk::Extent2D extent,
                    vk::ImageUsageFlags usage, vk::ImageLayout initialLayout, 
                    vk::ImageLayout finalLayout);

    void AddPass(const std::string& name,
                const std::vector<std::string>& inputs,
                const std::vector<std::string>& outputs,
                std::function<void(vk::raii::CommandBuffer&)> executeFunc);

    void Compile();

    Resource* GetResource(const std::string& name){
        auto it = resources.find(name);
        return (it != resources.end()) ? &it->second : nullptr;
    }

    void Execute(vk::raii::CommandBuffer& commandBuffer, vk::Queue queue);

private:
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};