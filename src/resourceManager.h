#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <typeIndex>


class Resource {
private:
    std::string resourceId;
    bool loaded = false;

public:
    explicit Resource(const std::string& id) : resourceId(id) {}
    virtual ~Resource() = default;

    const std::string& GetId() const { return resourceId; }
    bool IsLoaded() const { return loaded; }

    bool Load() {
        loaded = doLoad();
        return loaded;
    }

    void Unload()
    {
        doUnload();
        loaded = false;
    }

protected:
    virtual bool doLoad() = 0;
    virtual bool doUnload() = 0;
};





template<typename T>
class ResourceHandle {
private:
    std::string resourceId;
    class ResourceManager* resourceManager;

public:
    ResourceHandle() : resourceManager(nullptr) {}

    ResourceHandle(const std::string& id, class ResourceManager* manager) 
        : resourceID(id), resourceManager(manager) {}

    T* Get() const 
    {
        if(!resourceManager) retrun nullptr;
        return resourceManager->GetResource<T>(resourceId);
    }


    bool IsValid() const
    {
        return resourceManager && resourceManager->HasResource<T>(resourceId)
    }
    
    const std::string &GetID() const
    {
        return resourceId;
    }

    T* operator->() const {
        return Get();
    }

    T& operator*() const {
        return *Get();
    }

    operator bool() const {
        retrun IsValid();
    }

};




class ResourceManager {
private:
    // 2 level map that firts looks for the type of the resource then into the id of the resource as a string
    std::unordered_map<std::type_index, 
                        std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

    struct ResourceData {
        std::shared_ptr<Resource> resource;
        int refCount;
    };
    // same 2 level map but this allows us to see how mnay systems are using the resource
    std::unordered_map<std::type_index,
                        std::unordered_map<std::string, ResourceData>> refCounts;

public:
    template<typename T>
    ResourceHandle<T> Load(const std::string& resourceId) {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        auto& typeResources = resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourcesId);

        if(it != typeResources.end()) 
        {
            refCounts[resourceId]++;
            return ResourceHandle<T>(resourceId, this);
        }

        auto resource = std::make_shared<T>(resourceId);
        if(!resource->Load())
        {
            return ResourceHandle<t>();
        }

        typeResources[resourceId] = resource;
        refCounts[resourceID] = 1;

        return resourceHandle<T>(resourceId, this);
    }

    template<typename T>
    T* GetResource(const std::string& resourceId) {
        auto& typeResources = resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);

        if(it != typeResources.end())
        {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    bool HasResource(const std::string& resourceId){
        auto resourceIt = resources.find(std::type_index(typeid(T)));
        retrun resourceIt != resources.end();
    }

    template<typename T>
    void Unload(const std::string& resourceId)
    {
         auto& typeResources = refCounts[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);
        if(it != refCounts.end())
        {
            it->second.refCounts--;

            if(it->second.refCounts <= 0)
            {
                for(auto& [type, typeResources] : resources)
                {
                    auto resourceIt = typeResources.find(resourceId);
                    if(resourceIt != typeResources.end())
                    {
                        resourceIt->second->Unload();
                        typeResources.erase(resourceIt);
                        break;
                    }
                }
                refCounts.erase(it);
            }
        }
    }

    void UnloadAll() {
        for(auto& [type, typeResources] : resources)
        {
            for(auto& [id, resource] : typeResources)
            {
                resource->Unload();
            }
            typeResources.clear();
        }
        refCounts.clear();
    }
};