
#include "glfw3.h"

#include "vulkanWindow.h"
#include "serviceLocator.h"
#include "VulkanDevice.h"
#include "Renderer.h"


void Renderer::createSwapChain()
{
    auto* vulkanDevice = ServiceLocator::GetVulkanDevice();
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = vulkanDevice->getPhysicalDevice().getSurfaceCapabilitiesKHR(vulkanDevice->getSurface());
    swapchainExtent = chooseSwapExtent(surfaceCapabilities);
    minImageCount = chooseMinImageCount(surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> availableFormats = vulkanDevice->getPhysicalDevice().getSurfaceFormatsKHR(vulkanDevice->getSurface());
    swapchainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

    // continue
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