#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "ECSEngine.h"
#include "LocationComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics/Sprite.hpp>

namespace ECSEngine {

// layers go 0 to 2, see SpriteComponent
const int kSpriteLayerCount = 3;

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entityManager = engine->GetEntityManager();
        WindowManager& windowManager = engine->GetWindowManager();
        SpriteManager& spriteManager = engine->GetSpriteManager();
        ComponentStorage<SpriteComponent>& spriteStorage =
            entityManager.template GetComponentStorage<SpriteComponent>();

        std::shared_ptr<sf::RenderWindow> window = windowManager.GetWindow();
        window->clear();

        // one pass to sort the sprites by layer
        for (int layer = 0; layer < kSpriteLayerCount; layer++) {
            mSlotsInLayer[layer].clear();
        }
        for (size_t slot = 0; slot < spriteStorage.mStorage.size(); slot++) {
            if (!spriteStorage.mValid[slot]) {
                continue;
            }

            int layer = spriteStorage.mStorage[slot].layer;
            if (layer < kSpriteLayerCount) {
                mSlotsInLayer[layer].push_back(slot);
            }
        }

        // draw layer by layer so the background is under the world and the hud is on top
        for (int layer = 0; layer < kSpriteLayerCount; layer++) {
            for (size_t slot : mSlotsInLayer[layer]) {
                const SpriteComponent& spriteComponent = spriteStorage.mStorage[slot];
                EntityID owner = spriteStorage.mOwner[slot];
                if (!entityManager.template HasComponent<LocationComponent>(owner)) {
                    continue;
                }

                Point2D location =
                    entityManager.template GetComponent<LocationComponent>(owner).loc;
                Rect bounds = spriteComponent.bounds;
                Rect windowRect{
                    Point2D(location.x + bounds.topLeft.x, location.y + bounds.topLeft.y),
                    bounds.width, bounds.height};

                // screen space sprites (like the score) don't move with the camera
                if (spriteComponent.worldSpace) {
                    windowRect = windowManager.WorldToWindow(windowRect);
                }

                sf::Sprite& sprite = spriteManager.GetSprite(spriteComponent.textureID);
                DrawSprite(*window, sprite, windowRect);
            }
        }

        // the engine calls display() after all the systems
        return kContinue;
    }

private:
    // stretches the sprite to fill windowRect
    void DrawSprite(sf::RenderWindow& window, sf::Sprite& sprite, const Rect& windowRect) {
        sf::IntRect textureRect = sprite.getTextureRect();
        float scaleX = windowRect.width / static_cast<float>(textureRect.size.x);
        float scaleY = windowRect.height / static_cast<float>(textureRect.size.y);
        sprite.setScale({scaleX, scaleY});

        // whole pixels so the tiles line up without thin gaps between them
        float x = std::round(windowRect.topLeft.x);
        float y = std::round(windowRect.topLeft.y);
        sprite.setPosition({x, y});

        window.draw(sprite);
    }

    // slots of the sprites in each layer, refilled every frame
    std::vector<size_t> mSlotsInLayer[kSpriteLayerCount];
};

}  // namespace ECSEngine
