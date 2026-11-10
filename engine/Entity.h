#pragma once

#include <array>
#include <limits>
#include <string>
#include <tuple>

// An entity can have any component it is templated with,
// but does not have these installed by default.
// This is a base class for all entities in the ECS system.

namespace ECSEngine {

using EntityID = size_t;

// id of an empty slot (entity was removed)
inline constexpr EntityID kInvalidEntity = std::numeric_limits<EntityID>::max();

class Entity {
public:
    EntityID GetID() const { return mID; }
    bool IsValid() const { return mID != kInvalidEntity; }

private:
    Entity() = default;
    Entity(EntityID eid, const std::string& name) : mID(eid), mType(name) {}
    EntityID mID = kInvalidEntity;
    std::string mType;

    template <typename... T>
    friend class EntityManager;
};

}  // namespace ECSEngine
