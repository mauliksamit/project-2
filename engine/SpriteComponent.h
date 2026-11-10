#pragma once

#include <cstdint>

#include "MathUtil.h"
#include "SpriteManager.h"

namespace ECSEngine {

// Draw order used by SpriteSystem (lower layers are drawn first).
inline constexpr uint8_t kLayerBackground = 0;  // sky.map tiles
inline constexpr uint8_t kLayerWorld = 1;       // world.map tiles, player, stars
inline constexpr uint8_t kLayerHUD = 2;         // score digits

// worldSpace == true:  bounds are world units relative to LocationComponent::loc
//                      (needs a LocationComponent).
// worldSpace == false: bounds are window pixels, the camera does not move it (HUD).
struct SpriteComponent {
    SpriteID textureID = 0;
    Rect bounds;
    bool worldSpace = true;
    uint8_t layer = kLayerWorld;
};

}  // namespace ECSEngine
