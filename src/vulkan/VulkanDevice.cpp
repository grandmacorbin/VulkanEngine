#include <iostream>
#include <map>
#include <ranges>
#define GLFW_INCLUDE_VULKAN

#include "VulkanDevice.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

std::vector<const char *>					requiredDeviceExtension = {
	vk::KHRSynchronization2ExtensionName,
    vk::KHRSwapchainExtensionName,
};

VulkanDevice::BufferAllocation VulkanDevice::CreateBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties
    )
    {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);

        vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
        buffer.bindMemory(*bufferMemory, 0);
        return {std::move(buffer), std::move(bufferMemory)};
    }


VulkanDevice::ImageAllocation VulkanDevice::CreateImage(
        uint32_t width, uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties
    )
    {
        vk::Extent3D newExtent;
        newExtent.width = width;
        newExtent.height = height;
        newExtent.depth = 1;

        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.format = format;
        imageInfo.extent = newExtent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.usage = usage;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::raii::Image image = vk::raii::Image(device, imageInfo);

        vk::MemoryRequirements memRequirments = image.getMemoryRequirements();

    }

vk::raii::CommandBuffer VulkanDevice::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo;
}

void VulkanDevice::EndSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer)
{

}

void VulkanDevice::createInstance()
{
    // TODO add validation layers
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "VulkanEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "vkSlayer";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = vk::ApiVersion14;

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.pApplicationInfo = &appInfo;
    
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for(uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        if(std::ranges::none_of(extensionProperties,
        [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
        { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    instance = vk::raii::Instance(context, createInfo);
}

void VulkanDevice::setupDebugMessenger()
{
    if(!enableValidationLayers)
    {
        return;
    }

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
    debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
    debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
    debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void VulkanDevice::createSurface(Window& window)
{
    VkSurfaceKHR _surface;
    if(glfwCreateWindowSurface(*instance, window.getHandle(), nullptr, &_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to creat window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance, _surface);
}
    
void VulkanDevice::pickPhysicalDevice()
{
    auto physicalDevices = instance.enumeratePhysicalDevices();

    auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
    if(devIter == physicalDevices.end())
    {
        throw std::runtime_error("failed to find GPUs with Vulkan Support!");
    }

    physicalDevice = *devIter;
    // TODO add MSSA sample count and features
}

void VulkanDevice::createLogicalDevice()
{
    
}

void VulkanDevice::retrieveQueues()
{

}

void VulkanDevice::createCommandPools()
{

}

void VulkanDevice::createAllocator()
{

}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{

}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDevice::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

bool VulkanDevice::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
    // TODO add features as we add more features to engine
    bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

    auto deviceProperties = physicalDevice.getProperties();
    auto deviceFeatures = physicalDevice.getFeatures();
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    bool supportGraphicsAndCompute = 
        std::ranges::any_of(queueFamilies, [](auto const &qfp) 
        { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics && qfp.queueFlags & vk::QueueFlagBits::eCompute); });

    bool supportsAllRequiredExtensions =
        std::ranges::all_of(requiredDeviceExtension,
            [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
                return std::ranges::any_of(availableDeviceExtensions,
                    [requiredDeviceExtension](auto const& availableDeviceExtension) { std::cout << availableDeviceExtension.extensionName << std::endl; return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
            });
    
    auto features = physicalDevice.template getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        >();
    bool supportsRequiredFeatures =
        features.template get<
        vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    return supportGraphicsAndCompute && supportsVulkan1_3 && supportsAllRequiredExtensions && supportsRequiredFeatures;
}