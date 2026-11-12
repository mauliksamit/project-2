#pragma once

#include <string>

#include "Game.h"
#include "SpriteManager.h"

// Loads a .map file (format in the assignment) and creates one entity per tile.
//   dictionary 1  -> background tiles: sprite only, kLayerBackground
//   dictionary 2  -> world tiles: sprite + static CollisionComponent, kLayerWorld
//                    'x' tiles damage the player, 'S' tiles become star spawners
// '.' and any character not in the dictionary leave the cell empty.
// Each dictionary sprite is registered with the SpriteManager once and shared by all
// tiles that use it. Prints an error and returns false if the file is missing or malformed.
bool LoadMap(Engine& engine, const std::string& path, const std::string& file,
             ECSEngine::SpriteID starSprite);
