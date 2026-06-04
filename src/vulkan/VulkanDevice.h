#include <vector>

#include "window.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_raii.hpp"



class VulkanDevice {
private:
    vk::raii::Context context;
#define NDEBUG
    #ifdef NDEBUG
    bool enableValidationLayers = false;
    #else
    bool enableValidationLayers = true;
    #endif
    vk::DebugUtilsMessengerEXT debugMessenger;

    vk::raii::Instance instance = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;

    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device;

    vk::Queue graphicsComputeQueue;
    vk::Queue presentQueue;

    uint32_t graphicsComputeFamily; // same family for both right now
    uint32_t presentFamily;

    vk::raii::CommandPool graphicsCommandPool = nullptr;

public:
    void initialize(Window& window);
    void shutdown();

    vk::raii::PhysicalDevice& getPhysicalDevice()
    {
        return physicalDevice;
    }
    vk::raii::Device& getDevice()
    {
        return device;
    }


    struct BufferAllocation {
        vk::raii::Buffer buffer;
        vk::raii::DeviceMemory memory;
    };

    BufferAllocation CreateBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );

    struct ImageAllocation {
        vk::raii::Image image;
        vk::raii::DeviceMemory memory;
        vk::raii::ImageView view;
    };

    ImageAllocation CreateImage(
        uint32_t width, uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );

    vk::raii::CommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);

private:
    void createInstance();

    void setupDebugMessenger();

    void createSurface(Window& window);
    
    void pickPhysicalDevice();

    void createLogicalDevice();

    void retrieveQueues();

    void createCommandPools();

    void createAllocator();

    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

    bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
};
