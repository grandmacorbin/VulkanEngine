#include "componentTypeIDSystem.h"


class Entity;

size_t ComponentTypeIDSystem::nextTypeID = 0;

class Component {
public:
    enum class State {
        Uninitialized,
        Initializing,
        Active,
        Destroying,
        Destroyed
    };

protected:
    State state = State::Uninitialized;
    Entity* owner = nullptr;

public:
    virtual ~Component() {
        if(state != State::Destroyed)
        {
            OnDestroy();
            state = State::Destroyed;
        }
    }

    template<typename T>
    static size_t GetTypeID(){
        return ComponentTypeIDSystem::GetTypeID<T>();
    }

    void Initialize() {
        if(state == State::Uninitialized) {
            state = State::Initializing;
            OnInitialize();
            state = State::Active;
        }
    }

    void Destroy() {
        if(state == State::Active)
        {
            state = State::Destroying;
            OnDestroy();
            state = State::Destroyed;
        }
    }

    bool IsActive() const { return state == State::Active; }

    void SetOwner(Entity* entity) { owner = entity; }
    Entity* GetOwner() const { return owner; }

protected:
    virtual void OnInitialize() {}
    virtual void OnDestroy() {}
    virtual void Update(float deltaTime) {}
    virtual void Render() {}

    friend class Entity;

};