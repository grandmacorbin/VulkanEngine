#include "window.h"

void Window::initialize(int width, int height, const std::string& title)
{
    

    this->width = width;
    this->height = height;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if(!window)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
}

void Window::shutdown()
{
    if(!window)
    {
        return;
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}

void Window::pollEvents()
{
    glfwPollEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

GLFWwindow* Window::getHandle() const
{
    if(!window)
    {
        throw std::runtime_error("Window not initialized");
    }

    return window;
}

vk::Extent2D Window::getExtent() const
{
    int frameBufferWidth;
    int frameBufferHeight;

    glfwGetFramebufferSize(
        window,
        &frameBufferWidth,
        &frameBufferHeight
    );

    return vk::Extent2D{
        static_cast<uint32_t>(frameBufferWidth),
        static_cast<uint32_t>(frameBufferHeight)
    };
}

bool Window::windowResized() const
{
    return frameBufferResized;
}

void Window::resetWindowResizedFlag()
{
    frameBufferResized = false;
}

void Window::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto appWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    appWindow->frameBufferResized = true;

    appWindow->width = width;
    appWindow->height = height;
}