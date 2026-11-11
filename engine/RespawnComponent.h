#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Where the entity goes back to (its LocationComponent::loc) when it touches something
// with doesDamage set. CollisionSystem does the teleport and zeroes the velocity.
struct RespawnComponent {
    Point2D loc;
};

}  // namespace ECSEngine
