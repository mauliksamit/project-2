#pragma once

#include "MathUtil.h"

namespace ECSEngine {

// on the camera entity. CollisionSystem adds it only on the frame the player hits something
// (not every frame they're touching), CameraSystem shakes the camera and counts frameLimit down
struct CameraShakeComponent {
    int frameLimit;
    Point2D offset;  // world units, first frame moves the view by offset then it flips
};

}  // namespace ECSEngine
