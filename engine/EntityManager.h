#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Component.h"
#include "Entity.h"

namespace ECSEngine {

// owns all the entities and their components
// don't keep references or iterators from here, adding entities or components can break them
// keep the EntityID and look it up again instead
template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();

    // returns the new entity's id, ids of removed entities get reused
    [[nodiscard]] EntityID CreateEntity(const std::string& name) {
        if (!mFreeEntityIDs.empty()) {
            EntityID reusedID = mFreeEntityIDs.back();
            mFreeEntityIDs.pop_back();
            mEntities[reusedID] = Entity(reusedID, name);
            return reusedID;
        }

        EntityID newID = mEntities.size();
        mEntities.push_back(Entity(newID, name));
        return newID;
    }

    // also removes all its components, does nothing if it's already removed
    void RemoveEntity(EntityID entity) {
        if (!EntityExists(entity)) {
            return;
        }

        // runs RemoveComponent for each component type
        (RemoveComponent<Components>(entity), ...);

        mEntities[entity] = Entity();
        mFreeEntityIDs.push_back(entity);
    }

    // stores a copy, replaces the old one if the entity already has this component
    template <typename T>
    void AddComponent(EntityID entity, T component) {
        assert(EntityExists(entity));
        ComponentStorage<T>& storage = GetComponentStorage<T>();

        if (HasComponent<T>(entity)) {
            size_t existingSlot = storage.mEntityToSlot[entity];
            storage.mStorage[existingSlot] = component;
            return;
        }

        if (entity >= storage.mEntityToSlot.size()) {
            storage.mEntityToSlot.resize(mEntities.size(), kNullComponent);
        }

        size_t slot;
        if (storage.mFreeList.empty()) {
            slot = storage.mStorage.size();
            storage.mStorage.push_back(component);
            storage.mValid.push_back(true);
            storage.mOwner.push_back(entity);
        } else {
            slot = storage.mFreeList.back();
            storage.mFreeList.pop_back();
            storage.mStorage[slot] = component;
            storage.mValid[slot] = true;
            storage.mOwner[slot] = entity;
        }
        storage.mEntityToSlot[entity] = slot;
    }

    // does nothing if the entity doesn't have this component
    template <typename T>
    void RemoveComponent(EntityID entity) {
        if (!HasComponent<T>(entity)) {
            return;
        }

        ComponentStorage<T>& storage = GetComponentStorage<T>();
        size_t slot = storage.mEntityToSlot[entity];
        storage.mValid[slot] = false;
        storage.mOwner[slot] = kInvalidEntity;
        storage.mFreeList.push_back(slot);
        storage.mEntityToSlot[entity] = kNullComponent;
    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        const ComponentStorage<T>& storage = std::get<ComponentStorage<T>>(mComponentStorages);

        // this entity never got this component type
        if (entity >= storage.mEntityToSlot.size()) {
            return false;
        }
        return storage.mEntityToSlot[entity] != kNullComponent;
    }

    // entity has to have this component, so check HasComponent first
    // reference is good until another T gets added or this one gets removed
    template <typename T>
    T& GetComponent(EntityID entity) {
        assert(HasComponent<T>(entity));
        ComponentStorage<T>& storage = GetComponentStorage<T>();
        size_t slot = storage.mEntityToSlot[entity];
        return storage.mStorage[slot];
    }

    // lives as long as the manager does. removed components stay in it,
    // so check mValid when looping over mStorage
    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mComponentStorages);
    }

    // removed entities leave gaps, so check IsValid() when iterating
    using tEntity = std::vector<Entity>;
    using const_iterator = tEntity::const_iterator;
    const_iterator cbegin() const { return mEntities.begin(); }
    const_iterator cend() const { return mEntities.end(); }
    const_iterator begin() { return mEntities.begin(); }
    const_iterator end() { return mEntities.end(); }

private:
    bool EntityExists(EntityID entity) const {
        if (entity >= mEntities.size()) {
            return false;
        }
        return mEntities[entity].IsValid();
    }

    tEntity mEntities;
    std::vector<EntityID> mFreeEntityIDs;  // ids of removed entities
    std::tuple<ComponentStorage<Components>...> mComponentStorages;  // one per component type
};

}  // namespace ECSEngine
