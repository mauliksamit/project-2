#pragma once

#include <cstdint>

#include "MathUtil.h"
#include "SpriteManager.h"

namespace ECSEngine {

// Draw order used by SpriteSystem (lower layers are drawn first).
inline constexpr uint8_t kLayerBackground = 0;  // sky.map tiles
inline constexpr uint8_t kLayerWorld = 1;       // world.map tiles, player, stars
inline constexpr uint8_t kLayerHUD = 2;         // score digits

// bounds are relative to LocationComponent::loc. Every sprite needs a LocationComponent,
// HUD sprites included, or SpriteSystem skips it.
// worldSpace == true:  loc + bounds is in world units and moves with the camera.
// worldSpace == false: loc + bounds is in window pixels, the camera doesn't move it (HUD).
struct SpriteComponent {
    SpriteID textureID = 0;
    Rect bounds;
    bool worldSpace = true;
    uint8_t layer = kLayerWorld;
};

}  // namespace ECSEngine
