#include <string>

#include "glfw3.h"
#include "vulkan_raii.hpp"

class Window {
private:
    GLFWwindow* window = nullptr;

    bool frameBufferResized = false;
    uint32_t width;
    uint32_t height;

public:
    void initialize(int width, int height, const std::string& title);
    void shutdown();

    void pollEvents();

    bool shouldClose() const;

    GLFWwindow* getHandle() const;

    vk::Extent2D getExtent() const;

    bool windowResized() const;

    void resetWindowResizedFlag();

    static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
};