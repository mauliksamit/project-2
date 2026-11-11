#pragma once

#include <string>

#include "Entity.h"
#include "SpriteManager.h"

namespace ECSEngine {

// Put on a spawner entity (the 'S' tile). SpawnSystem counts timeToNextSpawn down
// every frame and creates a new entity of spawnType when it reaches zero.
// All times are in frames (see kFramesPerSecond in Constants.h).
struct SpawnComponent {
    EntityID entityID = kInvalidEntity;  // the spawner entity this component is on
    std::string spawnType;               // what to spawn, currently only "star"
    SpriteID spawnTextureID = 0;         // sprite given to each spawned entity
    int timeToNextSpawn = 0;
    int timeBetweenSpawns = 0;
    int totalSpawns = 0;      // spawns left, the spawner stops at 0
    float spawnSpeed = 4.0f;  // speed of spawned entities in px/frame, direction is random
};

}  // namespace ECSEngine
