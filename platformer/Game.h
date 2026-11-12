#pragma once

#include "ECSEngine.h"

// The full component list for the platformer. Shared by main.cpp and MapLoader.cpp
// so both see the same Engine type.
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollowerComponent, ECSEngine::InputComponent,                         \
        ECSEngine::GravityComponent, ECSEngine::CameraShakeComponent,                          \
        ECSEngine::ScoreComponent, ECSEngine::RespawnComponent

using Engine = ECSEngine::ECSEngine<COMPONENTS>;
