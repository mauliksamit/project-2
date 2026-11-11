#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// Top-left world position of an entity. Every other position (sprite bounds,
// collision box) is stored relative to this point, so moving loc moves everything.
// Written by MovementSystem and CollisionSystem; read by most other systems.
struct LocationComponent {
    Point2D loc;
};

}  // namespace ECSEngine
