#include <array>
#include <iostream>

#include "CameraSystem.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "Game.h"
#include "GameObjects.h"
#include "GravitySystem.h"
#include "InputSystem.h"
#include "MapLoader.h"
#include "MovementSystem.h"
#include "ScoreSystem.h"
#include "SpawnSystem.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";

namespace {

constexpr unsigned int kWindowWidth = 1024;
constexpr unsigned int kWindowHeight = 768;
constexpr float kTile = 64.0f;

const char* kTileSheet = "spritesheet-tiles-default.png";
const char* kCharacterSheet = "spritesheet-characters-default.png";
const ECSEngine::Rect kStarSource(640, 320, 64, 64);
const ECSEngine::Rect kPlayerSource(0, 0, 128, 128);

// digits 0..9 are stacked in one column of the tile sheet, 0 at the bottom
ECSEngine::Rect DigitSource(int digit) {
    return ECSEngine::Rect(832.0f, 832.0f - 64.0f * static_cast<float>(digit), 64.0f, 64.0f);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    Engine engine(kWindowWidth, kWindowHeight, "Platformer");
    auto& em = engine.GetEntityManager();
    auto& sprites = engine.GetSpriteManager();

    // Load sounds
    auto& sounds = engine.GetSoundManager();
    sounds.RegisterSound(gResourcePath + "sfx_jump.ogg", ECSEngine::kSoundJump);
    sounds.RegisterSound(gResourcePath + "footstep_grass_003.ogg", ECSEngine::kSoundLand);
    sounds.RegisterSound(gResourcePath + "sfx_gem.ogg", ECSEngine::kSoundStar);
    sounds.RegisterSound(gResourcePath + "footstep_snow_001.ogg", ECSEngine::kSoundWallSlide);

    // Load maps (sky first so it is created on the background layer)
    ECSEngine::SpriteID starSprite = sprites.RegisterTexture(gResourcePath + kTileSheet, kStarSource);
    if (!LoadMap(engine, gResourcePath, "sky.map", starSprite) ||
        !LoadMap(engine, gResourcePath, "world.map", starSprite)) {
        return 1;
    }

    // Place player on the first floor (row 10 of world.map), inside the first walls
    ECSEngine::SpriteID playerSprite =
        sprites.RegisterTexture(gResourcePath + kCharacterSheet, kPlayerSource);
    ECSEngine::EntityID player =
        ECSEngine::CreatePlayer(&engine, ECSEngine::Point2D(13 * kTile, 9 * kTile), playerSprite);

    // The map has no star past the spike walls, so add one between columns 50 and 53
    ECSEngine::CreateStar(&engine, ECSEngine::Point2D(51.5f * kTile, 10.0f * kTile),
                          ECSEngine::Point2D(0.0f, 0.0f), starSprite);

    // Setup score component (upper-left corner, screen space)
    std::array<ECSEngine::SpriteID, 10> digitSprites{};
    for (int d = 0; d < 10; ++d) {
        digitSprites[static_cast<std::size_t>(d)] =
            sprites.RegisterTexture(gResourcePath + kTileSheet, DigitSource(d));
    }
    ECSEngine::CreateScoreDisplay(&engine, digitSprites, ECSEngine::Point2D(16.0f, 16.0f));

    // Setup Camera: fixed y, follows the player in x (CameraSystem uses the exclusion zones)
    const float w = static_cast<float>(kWindowWidth);
    const float h = static_cast<float>(kWindowHeight);
    ECSEngine::EntityID camera = em.CreateEntity("camera");
    ECSEngine::CameraComponent cam;
    cam.position = ECSEngine::Point2D(13 * kTile, 7 * kTile);
    cam.worldUnitsPerPixel = 1.0f;
    cam.hardExclusion = ECSEngine::Rect(0.1f * w, 0.0f, 0.8f * w, h);  // never in outer 10%
    cam.softExclusion = ECSEngine::Rect(0.3f * w, 0.0f, 0.4f * w, h);  // settle in centre 40%
    cam.speed = 0.08f;
    em.AddComponent(camera, cam);
    em.AddComponent(camera, ECSEngine::CameraFollowerComponent{player});
    em.AddComponent(camera, ECSEngine::CameraShakeComponent{0, ECSEngine::Point2D(0.0f, 0.0f)});

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
