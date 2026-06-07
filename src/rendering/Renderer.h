

#include "vulkan/vulkan_raii.hpp"


class Renderer {
public:
    vk::raii::SwapchainKHR swapchain = nullptr;
    std::vector<vk::Image> swapchainImages;
    vk::SurfaceFormatKHR swapchainSurfaceFormat;
    vk::Extent2D swapchainExtent;
    std::vector<vk::raii::ImageView> swapchainImageViews;
    uint32_t minImageCount;





private:
    void createSwapChain();
    void createImageViews();

    void createGraphicsPipeline();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t chooseMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

};