#pragma once

#include <vector>

#include "Entity.h"

namespace ECSEngine {

template <typename T>
class ComponentStorage {
public:
    std::vector<T> mStorage;
    std::vector<bool> mValid;           // Quickly track if a component is valid
    std::vector<EntityID> mOwner;       // Which entity each component belongs to
    std::vector<size_t> mFreeList;      // Avoid scanning for free slots
    std::vector<size_t> mEntityToSlot;  // Where each entity's component is in mStorage
};

}  // namespace ECSEngine
