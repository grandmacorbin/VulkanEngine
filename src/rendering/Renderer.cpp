
#include "glfw3.h"

#include "vulkanWindow.h"
#include "serviceLocator.h"
#include "VulkanDevice.h"
#include "Renderer.h"


void Renderer::createSwapChain()
{
    auto* vulkanDevice = ServiceLocator::GetVulkanDevice();
    auto support = vulkanDevice->querySwapchainSupport();

    swapchainExtent = chooseSwapExtent(support.capabilities);
    swapchainSurfaceFormat = chooseSwapSurfaceFormat(support.formats);
    minImageCount = chooseMinImageCount(support.capabilities);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.surface = vulkanDevice->getSurface();
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.imageFormat = swapchainSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchainSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapchainCreateInfo.preTransform = support.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCreateInfo.presentMode = chooseSwapPresentMode(support.presentModes);
    swapchainCreateInfo.clipped = true;

    swapchain = vk::raii::SwapchainKHR(vulkanDevice->getDevice(), swapchainCreateInfo);
    swapchainImages = swapchain.getImages();
}

void Renderer::createImageViews()
{
    assert(swapchainImageViews.empty());
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = swapchainSurfaceFormat.format;
    imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }; // level and layer count = 1 

    auto* vulkanDevice = ServiceLocator::GetVulkanDevice();
    for(auto &image : swapchainImages)
    {
        imageViewCreateInfo.image = image;
        swapchainImageViews.emplace_back(vulkanDevice->getDevice(), imageViewCreateInfo);
    }
}

vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &format)
        { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR Renderer::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {return presentMode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value;  }) ?
        vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
{
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    auto* window = ServiceLocator::Get<VulkanWindow>();
    glfwGetFramebufferSize(window->getHandle(), &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(width, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t Renderer::chooseMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
    {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}