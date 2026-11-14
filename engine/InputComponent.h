#pragma once

#include <bitset>

#include "Keyboard.h"

namespace ECSEngine {

struct InputComponent {
    int lastDash = 0;  // frames since the last dash
    std::bitset<Key::ScanCount> keydown;
    std::bitset<Key::ScanCount> lastKeydown;
};

}  // namespace ECSEngine
