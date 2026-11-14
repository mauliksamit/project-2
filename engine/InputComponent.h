#pragma once

#include <bitset>

#include "Keyboard.h"

namespace ECSEngine {

struct InputComponent {
    int lastDash = 0;  // frames since the last dash
    // set by a wall jump: air speed stays at kWallJumpVelocity until you land, touch a wall
    // or dash, so wide gaps (the second spike walls) need a dash
    bool wallJumpCap = false;
    std::bitset<Key::ScanCount> keydown;
    std::bitset<Key::ScanCount> lastKeydown;
};

}  // namespace ECSEngine
