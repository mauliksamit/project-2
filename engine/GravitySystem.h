#pragma once

#include "CollisionComponent.h"
#include "Constants.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "MovementComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// y goes down, so a positive y velocity means falling
const float kFallMultiplier = 2;      // falling is faster than going up
const float kFastFallMultiplier = 3;  // when holding down
// max fall speeds, kept under the spikes' height (44) so nothing falls through them
const float kMaxFallVelocity = 30;
const float kMaxFastFallVelocity = 40;
const float kWallSlideVelocity = 4;  // max fall speed while pushing into a wall

template <typename... Components>
class GravitySystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("GravitySystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entityManager = engine->GetEntityManager();
        ComponentStorage<GravityComponent>& gravityStorage =
            entityManager.template GetComponentStorage<GravityComponent>();

        for (size_t slot = 0; slot < gravityStorage.mStorage.size(); slot++) {
            if (!gravityStorage.mValid[slot]) {
                continue;
            }

            EntityID owner = gravityStorage.mOwner[slot];
            if (!entityManager.template HasComponent<MovementComponent>(owner)) {
                continue;
            }

            // InputSystem can turn gravity off for a few frames (like during a dash)
            GravityComponent& gravity = gravityStorage.mStorage[slot];
            if (gravity.disabledTimer > 0) {
                gravity.disabledTimer--;
                continue;
            }

            Point2D& velocity =
                entityManager.template GetComponent<MovementComponent>(owner).velocity;
            bool falling = velocity.y > 0;

            float multiplier = 1;
            float maxFallVelocity = kMaxFallVelocity;
            // only the player falls faster than it goes up, stars bounce evenly
            if (falling && entityManager.template HasComponent<InputComponent>(owner)) {
                multiplier = kFallMultiplier;
                if (IsKeyDown(entityManager, owner, Key::Down)) {
                    multiplier = kFastFallMultiplier;
                    maxFallVelocity = kMaxFastFallVelocity;
                }
            }

            velocity.x += gravity.acceleration.x;
            velocity.y += gravity.acceleration.y * multiplier;
            if (velocity.y > maxFallVelocity) {
                velocity.y = maxFallVelocity;
            }

            if (falling && IsPushingIntoWall(entityManager, owner)) {
                if (velocity.y > kWallSlideVelocity) {
                    velocity.y = kWallSlideVelocity;
                }

                sf::Sound& wallSlideSound = engine->GetSoundManager().GetSound(kSoundWallSlide);
                if (wallSlideSound.getStatus() != sf::Sound::Status::Playing) {
                    wallSlideSound.play();
                }
            }
        }
        return kContinue;
    }

private:
    // false if the entity has no InputComponent
    bool IsKeyDown(EntityManager<Components...>& entityManager, EntityID entity,
                   unsigned int key) {
        if (!entityManager.template HasComponent<InputComponent>(entity)) {
            return false;
        }
        return entityManager.template GetComponent<InputComponent>(entity).keydown.test(key);
    }

    // uses last frame's collision flags
    bool IsPushingIntoWall(EntityManager<Components...>& entityManager, EntityID entity) {
        if (!entityManager.template HasComponent<CollisionComponent>(entity)) {
            return false;
        }

        CollisionComponent& collision =
            entityManager.template GetComponent<CollisionComponent>(entity);
        // standing on the ground next to a wall isn't sliding down it
        if (collision.onGround) {
            return false;
        }

        bool pushingLeft = collision.onLeftWall && IsKeyDown(entityManager, entity, Key::Left);
        bool pushingRight = collision.onRightWall && IsKeyDown(entityManager, entity, Key::Right);
        return pushingLeft || pushingRight;
    }
};

}  // namespace ECSEngine
