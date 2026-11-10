#pragma once

#include <cstdint>

#include "MathUtil.h"

namespace ECSEngine {

// Axis-aligned collision data for an entity that also has a LocationComponent.
//
// collisionBox         relative to LocationComponent::loc.
// previousCollisionBox WORLD space. CollisionLateSystem writes it at the start of each
//                      frame, before anything moves, so CollisionSystem knows where the
//                      entity came from.
// on* flags            cleared and set by CollisionSystem every frame, so InputSystem and
//                      GravitySystem (which run before it) see last frame's contacts.
struct CollisionComponent {
    CollisionComponent()
        : staticEntity(0),
          doesDamage(0),
          collectable(0),
          onGround(0),
          onCeiling(0),
          onLeftWall(0),
          onRightWall(0) {}

    // Solid tile that never moves.
    static CollisionComponent Static(const Rect& box, bool damages = false) {
        CollisionComponent c;
        c.staticEntity = 1;
        c.doesDamage = damages ? 1 : 0;
        c.collisionBox = box;
        return c;
    }

    // Moving entity. restitution = fraction of speed kept when bouncing off a static.
    static CollisionComponent Dynamic(const Rect& box, float restitution = 0.0f,
                                      bool collectable = false) {
        CollisionComponent c;
        c.collectable = collectable ? 1 : 0;
        c.restitution = restitution;
        c.collisionBox = box;
        return c;
    }

    uint8_t staticEntity : 1;  // never moves, dynamic entities are pushed out of it
    uint8_t doesDamage : 1;    // touching it sends the player to its RespawnComponent
    uint8_t collectable : 1;   // the player picks it up on contact (stars)
    uint8_t onGround : 1;
    uint8_t onCeiling : 1;
    uint8_t onLeftWall : 1;
    uint8_t onRightWall : 1;
    float restitution = 0.0f;   // 0 = stop dead (player), 0.6 = bouncy (stars)
    Rect collisionBox;          // relative to entity location
    Rect previousCollisionBox;  // world space, last frame
};

}  // namespace ECSEngine
