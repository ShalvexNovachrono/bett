#ifndef Bett
#define Bett

#include "Logger.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

using EntityID = uint32_t;
static constexpr EntityID NULL_ENTITY = UINT32_MAX;

class CBasicComponentContainerFoundation {
public:
    virtual ~CBasicComponentContainerFoundation() = default;
    virtual void Remove(EntityID id) = 0;
    virtual bool Has(EntityID id) const = 0;
};

template <typename T>
class CComponentContainer : public CBasicComponentContainerFoundation {
public:
    template <typename... Args>
    T& EmplaceComponent(EntityID id, Args&&... args) {
        assert(!Has(id) && "Entity already has this component");
        size_t index = components.size();
        components.emplace_back(std::forward<Args>(args)...);
        entityList.push_back(id);
        entityToIndex[id] = index;
        return components[index];
    }

    T& GetComponent(EntityID id) {
        assert(Has(id) && "Entity does not have this component");
        return components[entityToIndex.at(id)];
    }

    void Remove(EntityID id) override {
        assert(Has(id) && "Entity does not have this component");

        size_t indexToRemove = entityToIndex[id];
        size_t lastIndex     = components.size() - 1;

        // swap with last element to keep dense array packed if not already at the end
        if (indexToRemove != lastIndex) {
            EntityID lastEntity       = entityList[lastIndex];
            components[indexToRemove] = std::move(components[lastIndex]);
            entityList[indexToRemove] = lastEntity;
            entityToIndex[lastEntity] = indexToRemove;
        }

        components.pop_back();
        entityList.pop_back();
        entityToIndex.erase(id);
    }

    bool Has(EntityID id) const override {
        return entityToIndex.count(id) > 0;
    }

    std::vector<T>& All()               { return components; }
    std::vector<EntityID>& Entities()   { return entityList; }
private:
    std::vector<T> components;                          // Packed array storing the actual component data contiguously
    std::vector<EntityID> entityList;                   // Maps index in 'components' back to its EntityID
    std::unordered_map<EntityID, size_t> entityToIndex; // Maps EntityID to its index inside 'components'
};

class CBett;

class GameObject {
private:
    EntityID id;
    CBett& cb;
public:
    GameObject(EntityID ID, CBett& CB);

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args);

    template<typename T>
    T& GetComponent();

    template<typename T>
    bool Has();

    template<typename T>
    void RemoveComponent();

    EntityID ID() const { return id; }
};

class CBett {
private:
    EntityID nextEntityID = 0;
    std::vector<EntityID> discardedEntityId{};
    std::vector<EntityID> inUseEntityID{};
    std::unordered_map<std::type_index, std::unique_ptr<CBasicComponentContainerFoundation>> stores;

public:
    CBett() = default;

    GameObject CreateGameObject() {
        return {GiveNextAvailableEntityID(), *this};
    }

    bool DoesGameObjectExist(EntityID entityID) {
        return std::find(inUseEntityID.begin(), inUseEntityID.end(), entityID) != inUseEntityID.end();
    }

    bool DoesGameObjectExist(GameObject gameObject) {
        return DoesGameObjectExist(gameObject.ID());
    }

    void RemoveGameObject(EntityID entityID) {
        DestroyGameObject(entityID);
    }

    void RemoveGameObject(GameObject gameObject) {
        DestroyGameObject(gameObject.ID());
    }

    void RemoveGameObject(EntityID entityID, bool(*callback)(EntityID)) {
        if (DoesGameObjectExist(entityID)) {
            if (callback == nullptr || callback(entityID)) {
                DestroyGameObject(entityID);
            }
        }
    }

    void RemoveGameObject(GameObject gameObject, bool(*callback)(EntityID)) {
        RemoveGameObject(gameObject.ID(), callback);
    }

    const std::vector<EntityID>& AllEntities() const {
        return inUseEntityID;
    }

    std::vector<GameObject> AllGameObjects() {
        std::vector<GameObject> gms;
        gms.reserve(inUseEntityID.size());
        for (auto id : inUseEntityID) {
            gms.push_back({id, *this});
        }
        return gms;
    }

    void forEachGameObjects(void (*func)(GameObject)) {
        if (func == nullptr) return;
        for (auto ee : inUseEntityID) {
            func(GameObject(ee, *this));
        }
    }

    template <typename T, typename... Args>
    T& AddComponent(EntityID id, Args&&... args) {
        return GetStore<T>().template EmplaceComponent<Args...>(id, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent(EntityID id) {
        return GetStore<T>().GetComponent(id);
    }

    template<typename T>
    bool Has(EntityID id) {
        return GetStore<T>().Has(id);
    }

    template<typename T>
    void RemoveComponent(EntityID id) {
        GetStore<T>().Remove(id);
    }

    template<typename T>
    CComponentContainer<T>& Store() {
        return GetStore<T>();
    }
private:
    bool DestroyGameObject(EntityID entity) {
        if (entity >= nextEntityID) {
            BLogWarning("Can't Remove/Destroy GameObject that doesn't exist. (GameObject) EntityID: " << entity << ".");
            return false;
        }

        auto elementIndex = std::find(inUseEntityID.begin(), inUseEntityID.end(), entity);

        if (elementIndex == inUseEntityID.end()) {
            BLogWarning("GameObject is already destroyed. EntityID: " << entity << ".");
            return false;
        }

        // Clean up any components belonging to this entity across all stores
        for (auto& [type, store] : stores) {
            if (store->Has(entity)) {
                store->Remove(entity);
            }
        }

        *elementIndex = inUseEntityID.back();
        inUseEntityID.pop_back();

        discardedEntityId.push_back(entity);
        return true;
    }

    EntityID GiveNextAvailableEntityID() {
        EntityID id;
        if (!discardedEntityId.empty()) {
            id = discardedEntityId.back(); // gives the last item from the array
            discardedEntityId.pop_back();
        } else {
            id = nextEntityID++;
        }

        // it doesn't need to be in order
        inUseEntityID.push_back(id);
        return id;
    }

    template <typename T>
    CComponentContainer<T>& GetStore() {
        auto key = std::type_index(typeid(T));
        if (!stores.count(key)) {
            stores[key] = std::make_unique<CComponentContainer<T>>();
        }
        return static_cast<CComponentContainer<T>&>(*stores[key]);
    }
};

inline GameObject::GameObject(EntityID ID, CBett& CB) : id(ID), cb(CB) {}

template <typename T, typename... Args>
T& GameObject::AddComponent(Args&&... args) {
    return cb.AddComponent<T>(id, std::forward<Args>(args)...);
}

template <typename T>
T& GameObject::GetComponent() {
    return cb.GetComponent<T>(id);
}

template <typename T>
bool GameObject::Has() {
    return cb.Has<T>(id);
}

template <typename T>
void GameObject::RemoveComponent() {
    cb.RemoveComponent<T>(id);
}

#endif
