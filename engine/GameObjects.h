#pragma once

#include <array>
#include <string>

#include "CollisionComponent.h"
#include "Entity.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"

// Factory helpers that build the entities used by the platformer. The map loader,
// SpawnSystem and main.cpp all go through these so an entity type is set up in one place.
// They only use the EngineView getters, never a manager stored somewhere else.

namespace ECSEngine {

template <typename... Components>
class EngineView;

// Tuning constants for spawned/created objects (px and px/frame at 60 fps).
inline constexpr float kStarSize = 40.0f;
inline constexpr float kStarGravity = 0.3f;
inline constexpr float kStarRestitution = 0.6f;
inline constexpr float kPlayerGravity = 0.6f;  // base value, GravitySystem scales it
inline constexpr float kPlayerSize = 64.0f;
inline constexpr float kDigitSize = 40.0f;

// Decoration only: location + sprite, no collision.
template <typename... Components>
EntityID CreateTile(EngineView<Components...>* engine, const Point2D& pos, SpriteID sprite,
                    const Rect& bounds, uint8_t layer) {
    auto& em = engine->GetEntityManager();
    EntityID id = em.CreateEntity("tile");
    em.AddComponent(id, LocationComponent{pos});
    em.AddComponent(id, SpriteComponent{sprite, bounds, true, layer});
    return id;
}

// Solid world tile. box is relative to pos. Spikes pass damages = true.
template <typename... Components>
EntityID CreateSolidTile(EngineView<Components...>* engine, const Point2D& pos, SpriteID sprite,
                         const Rect& bounds, const Rect& box, bool damages) {
    EntityID id = CreateTile(engine, pos, sprite, bounds, kLayerWorld);
    engine->GetEntityManager().AddComponent(id, CollisionComponent::Static(box, damages));
    return id;
}

// Spawner tile: drawn like a tile but has no collision so stars can leave it.
template <typename... Components>
EntityID CreateSpawner(EngineView<Components...>* engine, const Point2D& pos, SpriteID sprite,
                       const Rect& bounds, SpriteID starSprite) {
    EntityID id = CreateTile(engine, pos, sprite, bounds, kLayerWorld);
    SpawnComponent spawn;
    spawn.entityID = id;
    spawn.spawnType = "star";
    spawn.spawnTextureID = starSprite;
    spawn.timeToNextSpawn = 60;   // first star after 1 s
    spawn.timeBetweenSpawns = 180;  // then every 3 s
    spawn.totalSpawns = 20;
    engine->GetEntityManager().AddComponent(id, spawn);
    return id;
}

// Collectable star that falls, bounces off tiles and other stars.
template <typename... Components>
EntityID CreateStar(EngineView<Components...>* engine, const Point2D& pos, const Point2D& velocity,
                    SpriteID sprite) {
    auto& em = engine->GetEntityManager();
    EntityID id = em.CreateEntity("star");
    em.AddComponent(id, LocationComponent{pos});
    em.AddComponent(id, MovementComponent{velocity});
    em.AddComponent(id, GravityComponent(Point2D(0.0f, kStarGravity)));
    em.AddComponent(id, SpriteComponent{sprite, Rect(0, 0, kStarSize, kStarSize), true,
                                        kLayerWorld});
    em.AddComponent(id, CollisionComponent::Dynamic(Rect(0, 0, kStarSize, kStarSize),
                                                    kStarRestitution, true));
    return id;
}

// The player. Collision box is a bit narrower than the sprite so walls feel fair.
template <typename... Components>
EntityID CreatePlayer(EngineView<Components...>* engine, const Point2D& pos, SpriteID sprite) {
    auto& em = engine->GetEntityManager();
    EntityID id = em.CreateEntity("player");
    em.AddComponent(id, LocationComponent{pos});
    em.AddComponent(id, MovementComponent{Point2D(0.0f, 0.0f)});
    em.AddComponent(id, GravityComponent(Point2D(0.0f, kPlayerGravity)));
    em.AddComponent(id, SpriteComponent{sprite, Rect(0, 0, kPlayerSize, kPlayerSize), true,
                                        kLayerWorld});
    em.AddComponent(id, CollisionComponent::Dynamic(Rect(14, 8, 36, 56)));
    em.AddComponent(id, InputComponent(id));
    em.AddComponent(id, RespawnComponent{pos});
    return id;
}

// Score entity plus one screen-space digit entity per ScoreComponent::kNumDigits.
// screenPos is the top-left of the first digit in window pixels.
template <typename... Components>
EntityID CreateScoreDisplay(EngineView<Components...>* engine,
                            const std::array<SpriteID, 10>& digitSprites,
                            const Point2D& screenPos) {
    auto& em = engine->GetEntityManager();
    ScoreComponent score;
    score.digitTextureID = digitSprites;
    for (std::size_t i = 0; i < ScoreComponent::kNumDigits; ++i) {
        EntityID digit = em.CreateEntity("scoreDigit");
        Rect bounds(screenPos.x + static_cast<float>(i) * kDigitSize, screenPos.y, kDigitSize,
                    kDigitSize);
        em.AddComponent(digit, SpriteComponent{digitSprites[0], bounds, false, kLayerHUD});
        score.digitID[i] = digit;
    }
    EntityID id = em.CreateEntity("score");
    em.AddComponent(id, score);
    return id;
}

}  // namespace ECSEngine
