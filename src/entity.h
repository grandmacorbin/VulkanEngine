#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "component.h"


class Entity {
private:
    std::string name;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;
    std::unordered_map<size_t, Component*> componentMap;

public:
    explicit Entity(const std::string& entityName) : name(entityName) {}

    virtual ~Entity() = default;

    const std::string& GetName() const { return name; }
    bool IsActive() const { return active; }
    void SetActive(bool isActive) { active = isActive; }

    void Initialize();

    void Update(float deltaTime);

    void Render();

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        size_t typeID = Component::GetTypeID<T>();

        auto it = componentMap.find(typeID);
        if(it != componentMap.end()) {
            return static_cast<T*>(it->second);
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();
        componentMap[typeID] = componentPtr;
        componentPtr->SetOwner(this);
        components.push_back((std::move(component)));
        return componentPtr;
    }

    template<typename T>
    T* GetComponent() 
    {
        size_t typeID = Component::GetTypeID<T>();
        auto it = componentMap.find(typeID);
        if(it != componentMap.end())
        {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    template<typename T>
    bool RemoveComponent()
    {
        size_t typeID = Component::GetTypeID<T>();
        auto it = componentMap.find(typeID);
        if(it != componentMap.end())
        {
            Component* componentPtr = it->second;
            componentMap.erase(it);

            for(auto compIt = components.begin(); compIt != components.end(); ++compIt)
            {
                if(compIt->get() == componentPtr)
                {
                    components.erase(compIt);
                    return truel
                }
            }
        }
        return false;
    }
};