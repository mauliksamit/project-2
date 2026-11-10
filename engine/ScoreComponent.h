#pragma once

#include <array>
#include <cstddef>

#include "Entity.h"
#include "SpriteManager.h"

namespace ECSEngine {

// One per game. CollisionSystem increments score when a star is collected;
// ScoreSystem points each digit entity's SpriteComponent at the right number sprite.
struct ScoreComponent {
    static constexpr std::size_t kNumDigits = 3;  // shows 000..999

    int score = 0;
    int displayedScore = -1;                // last value drawn, -1 forces the first update
    std::array<SpriteID, 10> digitTextureID{};       // sprite for each number 0..9
    std::array<EntityID, kNumDigits> digitID{};      // digit entities, most significant first
};

}  // namespace ECSEngine
