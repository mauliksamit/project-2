#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// rects are in screen pixels, only x and width get used. main.cpp has to set both
// (hard = middle 80%, soft = middle 40%) and the position, since its y never changes
struct CameraComponent {
    Point2D position;  // [world coordinates] world point at the center of the screen
    float worldUnitsPerPixel = 1.0f;
    Rect hardExclusion;
    Rect softExclusion;
    float speed = 0.05f;  // 0 to 1, how much of the gap to softExclusion it closes each frame
};

}  // namespace ECSEngine
