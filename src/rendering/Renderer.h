

#include "vulkan/vulkan_raii.hpp"


class Renderer {




private:
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    
};