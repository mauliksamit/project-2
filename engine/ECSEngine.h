#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "Constants.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

template <typename... Components>
class EngineView {
public:
    // these refs are good as long as the engine is alive
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
};

// debug keys: F1 collision boxes, F2 pause, F3 step one frame
template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

private:
    bool ProcessEvents();  // false when the window gets closed
    void HandleDebugKey(sf::Keyboard::Scancode scancode);
    void SetKeyDown(sf::Keyboard::Scancode scancode, bool isDown);
    void DebugDraw();

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // For debugging - pause and step the engine
    bool running, step, debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name), running(true), step(false), debug(false) {
    std::shared_ptr<sf::RenderWindow> window = mWindowManager.GetWindow();
    window->setFramerateLimit(kFramesPerSecond);
    window->setKeyRepeatEnabled(false);
}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {}

template <typename... Components>
void ECSEngine<Components...>::Run() {
    while (ProcessEvents()) {
        if (!running && !step) {
            // display() does the frame limit but it doesn't run while paused
            sf::sleep(sf::seconds(1.0f / kFramesPerSecond));
            continue;
        }
        step = false;

        if (debug) {
            DebugDraw();
        }

        if (!mSystemManager.Run(this)) {
            return;
        }
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    std::shared_ptr<sf::RenderWindow> window = mWindowManager.GetWindow();
    ComponentStorage<InputComponent>& inputStorage =
        mEntityManager.template GetComponentStorage<InputComponent>();

    // InputSystem compares these to see if a key was just pressed
    for (size_t slot = 0; slot < inputStorage.mStorage.size(); slot++) {
        if (inputStorage.mValid[slot]) {
            InputComponent& input = inputStorage.mStorage[slot];
            input.lastKeydown = input.keydown;
        }
    }

    while (const std::optional<sf::Event> event = window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window->close();
            return false;
        }

        const sf::Event::KeyPressed* pressed = event->getIf<sf::Event::KeyPressed>();
        if (pressed != nullptr) {
            HandleDebugKey(pressed->scancode);
            SetKeyDown(pressed->scancode, true);
        }

        const sf::Event::KeyReleased* released = event->getIf<sf::Event::KeyReleased>();
        if (released != nullptr) {
            SetKeyDown(released->scancode, false);
        }
    }
    return true;
}

template <typename... Components>
void ECSEngine<Components...>::HandleDebugKey(sf::Keyboard::Scancode scancode) {
    if (scancode == sf::Keyboard::Scancode::F1) {
        debug = !debug;
    }
    if (scancode == sf::Keyboard::Scancode::F2) {
        running = !running;
    }
    if (scancode == sf::Keyboard::Scancode::F3) {
        step = true;
    }
}

template <typename... Components>
void ECSEngine<Components...>::SetKeyDown(sf::Keyboard::Scancode scancode, bool isDown) {
    // Unknown is -1, which isn't a spot in the bitset
    if (scancode == sf::Keyboard::Scancode::Unknown) {
        return;
    }

    size_t keyIndex = static_cast<size_t>(scancode);
    ComponentStorage<InputComponent>& inputStorage =
        mEntityManager.template GetComponentStorage<InputComponent>();
    for (size_t slot = 0; slot < inputStorage.mStorage.size(); slot++) {
        if (inputStorage.mValid[slot]) {
            inputStorage.mStorage[slot].keydown.set(keyIndex, isDown);
        }
    }
}

// runs before the systems, so the boxes are one frame behind
template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {
    ComponentStorage<CollisionComponent>& collisionStorage =
        mEntityManager.template GetComponentStorage<CollisionComponent>();

    for (size_t slot = 0; slot < collisionStorage.mStorage.size(); slot++) {
        if (!collisionStorage.mValid[slot]) {
            continue;
        }

        EntityID owner = collisionStorage.mOwner[slot];
        if (!mEntityManager.template HasComponent<LocationComponent>(owner)) {
            continue;
        }

        Point2D location = mEntityManager.template GetComponent<LocationComponent>(owner).loc;
        Rect box = collisionStorage.mStorage[slot].collisionBox;
        Rect worldBox{Point2D(location.x + box.topLeft.x, location.y + box.topLeft.y), box.width,
                      box.height};
        mWindowManager.DebugFrameRect(mWindowManager.WorldToWindow(worldBox), 255, 0, 0);
    }
}

}  // namespace ECSEngine
