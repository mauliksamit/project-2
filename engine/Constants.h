#pragma once

namespace ECSEngine {

// game runs at a fixed fps, so velocities are pixels per frame and timers count frames
inline constexpr unsigned int kFramesPerSecond = 60;

// names used to register and play sounds
inline constexpr const char* kSoundJump = "jump";
inline constexpr const char* kSoundLand = "land";
inline constexpr const char* kSoundStar = "star";
inline constexpr const char* kSoundWallSlide = "wallSlide";

}  // namespace ECSEngine
