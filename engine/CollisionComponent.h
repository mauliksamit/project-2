#pragma once

#include <bitset>
#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

// on* flags are reset and set in CollisionSystem, so InputSystem and GravitySystem
// see last frame's flags
struct CollisionComponent {
    uint8_t staticEntity : 1;
    uint8_t doesDamage : 1;
    uint8_t onGround : 1;
    uint8_t onCeiling : 1;
    uint8_t onLeftWall : 1;
    uint8_t onRightWall : 1;
    Rect collisionBox;          // relative to entity location
    Rect previousCollisionBox;  // The previous bounding box for collision detection
};

}  // namespace ECSEngine
