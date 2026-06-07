#pragma once
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <string>

#include "vulkan/vulkan_raii.hpp"

class ServiceLocator {
private:
    static std::unordered_map<std::type_index, void*> services;
    static bool initialized;

public:
    ServiceLocator() = delete;
    ~ServiceLocator() = delete;

    static void Initialize();
    static void Shutdown();

    template<typename T>
    static void Provide(T* service){
        std::type_index typeId = std::type_idex(typeid(T));

        if(services.find(typeId) != services.end()){
            throw std::runtime_error("Service already registered: " + std::string(typeid(T).name()));
        }

        service[typeId] = service;
    }

    template<typename T>
    static T* Get() {
        std::type_index typeId = std::type_index(typeid(T));

        auto it = services.find(typeId);
        if(it == services.end()){
            throw std::runtime_error("service not found: " + std::string(typeid(T).name()));
        }

        return static_cast<T*>(it->second);
    }

    template<typename T>
    static bool Has() {
        std::type_index typeId = std::type_index(typeid(T));
        return services.find(typeId) != services.end();
    }

    template<typename T>
    static void Remove() {
        std::type_index typeId = std::type_index(typeid(T));
        services.erase(typeId);
    }

    static class VulkanInstance* GetVulkanInstance();
    static class VulkanDevice* GetVulkanDevice();
    static class VulkanDevice* GetDevice();
    static class Renderer* GetRenderer();
    static class ResourceManager* GetResourceManager();
    static class InputManager* GetInput();
    static class Window* GetWindow();
};