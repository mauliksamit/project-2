#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// on the camera entity. CollisionSystem sets it when the player hits something,
// CameraSystem shakes the camera and counts frameLimit down
struct CameraShakeComponent {
    int frameLimit;
    Point2D offset;
};

}  // namespace ECSEngine
