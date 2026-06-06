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

void VulkanDevice::initialize(Window &window)
{
    createInstance();
    setupDebugMessenger();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    retrieveQueues();
    createCommandPools();
    createAllocator();
}

void VulkanDevice::shutdown()
{
    //TODO
}

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
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    vk::raii::CommandBuffer commandBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

    commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;

    graphicsComputeQueue.submit(submitInfo, nullptr);
    graphicsComputeQueue.waitIdle();
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
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    uint32_t queueIndex = ~0;
    for(uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
        if((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && (queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eCompute)
            && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
            {
                queueIndex = qfpIndex;
                break;
            }
    }
    if(queueIndex == ~0)
    {
        throw std::runtime_error("Could not find a queue for graphics and compute and present -> terminating");
    }
    graphicsComputeFamily = queueIndex;
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.queueFamilyIndex = graphicsComputeFamily;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::StructureChain<vk::PhysicalDeviceFeatures2, 
        vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> 
        featureChain{};
    featureChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
    featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();

    device = vk::raii::Device( physicalDevice, deviceCreateInfo );
    
}

void VulkanDevice::retrieveQueues()
{
    graphicsComputeQueue = vk::raii::Queue( device, graphicsComputeFamily, 0);
}

void VulkanDevice::createCommandPools()
{
    vk::CommandPoolCreateInfo graphicsPoolInfo{};
    graphicsPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    graphicsPoolInfo.queueFamilyIndex = graphicsComputeFamily;

    graphicsCommandPool = vk::raii::CommandPool(device, graphicsPoolInfo);
}

void VulkanDevice::createAllocator()
{

}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
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