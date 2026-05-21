#include "serviceLocator.h"
#include "vulkan/VulkanDevice.h"
#include "resources/resourceManager.h"

std::unordered_map<std::type_index, void*> ServiceLocator::services;
bool ServiceLocator::initialized = false;

void ServiceLocator::Initialize(){
    if(initialized) {
        throw std::runtime_error("ServiceLocator already initialized!");
    }

    services.clear();
    initialized = true;
}

void ServiceLocator::Shutdown() {
    if(!initialized){
        return;
    }

    services.clear();
    initialized = false;
}

VulkanInstance* ServiceLocator::GetVulkanInstance() {
    return Get<VulkanInstance>();
}

VulkanPhysicalDevice* ServiceLocator::GetPhysicalDevice() {
    return Get<VulkanPhysicalDevice>();
}

VulkanDevice* ServiceLocator::GetDevice() {
    return Get<VulkanDevice>();
}

Renderer* ServiceLocator::GetRenderer() {
    return Get<Renderer>();
}

ResourceManager* ServiceLocator::GetResourceManager() {
    return Get<ResourceManager>();
}

InputManager* ServiceLocator::GetInput() {
    return Get<InputManager>();
}

Window* ServiceLocator::GetWindow() {
    return Get<Window>();
}