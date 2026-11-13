#pragma once

#include "CollisionComponent.h"
#include "Constants.h"
#include "ECSEngine.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "Keyboard.h"
#include "MathUtil.h"
#include "MovementComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

const float kMaxRunVelocity = 15;
const float kDashVelocity = 50;
const float kJumpVelocity = 25;
const float kGroundRunAcceleration = 5;
const float kAirRunAcceleration = 4;
const int kDashInterval = kFramesPerSecond;  // at most one dash a second
const int kDashGravityFrames = 8;            // no gravity for a bit so the dash stays level
const float kWallJumpVelocity = 12;          // sideways speed when jumping off a wall
const float kJumpCutMultiplier = 0.4f;       // letting go of z keeps this much of the jump

// arrows to run, z to jump, c to dash. uses last frame's collision flags
template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entityManager = engine->GetEntityManager();
        ComponentStorage<InputComponent>& inputStorage =
            entityManager.template GetComponentStorage<InputComponent>();

        for (size_t slot = 0; slot < inputStorage.mStorage.size(); slot++) {
            if (!inputStorage.mValid[slot]) {
                continue;
            }

            EntityID owner = inputStorage.mOwner[slot];
            if (!entityManager.template HasComponent<MovementComponent>(owner)) {
                continue;
            }

            InputComponent& input = inputStorage.mStorage[slot];
            Point2D& velocity =
                entityManager.template GetComponent<MovementComponent>(owner).velocity;

            bool onGround = false;
            bool onLeftWall = false;
            bool onRightWall = false;
            if (entityManager.template HasComponent<CollisionComponent>(owner)) {
                CollisionComponent& collision =
                    entityManager.template GetComponent<CollisionComponent>(owner);
                onGround = collision.onGround;
                onLeftWall = collision.onLeftWall;
                onRightWall = collision.onRightWall;
            }

            bool leftHeld = input.keydown.test(Key::Left);
            bool rightHeld = input.keydown.test(Key::Right);
            float runDirection = 0;
            if (leftHeld && !rightHeld) {
                runDirection = -1;
            }
            if (rightHeld && !leftHeld) {
                runDirection = 1;
            }

            // speed up or slow down a bit each frame instead of instantly
            float acceleration = kAirRunAcceleration;
            if (onGround) {
                acceleration = kGroundRunAcceleration;
            }
            velocity.x = MoveTowards(velocity.x, runDirection * kMaxRunVelocity, acceleration);

            // jump off the ground, or off a wall you're pushing into while falling
            if (JustPressed(input, Key::Z)) {
                bool falling = velocity.y > 0;
                if (onGround) {
                    velocity.y = -kJumpVelocity;
                    PlayJumpSound(engine);
                } else if (falling && onLeftWall && leftHeld) {
                    velocity.y = -kJumpVelocity;
                    velocity.x = kWallJumpVelocity;
                    PlayJumpSound(engine);
                } else if (falling && onRightWall && rightHeld) {
                    velocity.y = -kJumpVelocity;
                    velocity.x = -kWallJumpVelocity;
                    PlayJumpSound(engine);
                }
            }

            // letting go of z while going up cuts the jump short
            if (velocity.y < 0 && JustReleased(input, Key::Z)) {
                velocity.y *= kJumpCutMultiplier;
            }

            // lastDash counts frames since the last dash
            if (input.lastDash < kDashInterval) {
                input.lastDash++;
            }

            // dash the way you're holding, or the way you're already moving
            float dashDirection = runDirection;
            if (dashDirection == 0) {
                dashDirection = Sign(velocity.x);
            }
            if (JustPressed(input, Key::C) && input.lastDash >= kDashInterval &&
                dashDirection != 0) {
                velocity.x = dashDirection * kDashVelocity;
                velocity.y = 0;
                input.lastDash = 0;
                if (entityManager.template HasComponent<GravityComponent>(owner)) {
                    entityManager.template GetComponent<GravityComponent>(owner).disabledTimer =
                        kDashGravityFrames;
                }
            }
        }
        return kContinue;
    }

private:
    bool JustPressed(const InputComponent& input, unsigned int key) {
        return input.keydown.test(key) && !input.lastKeydown.test(key);
    }

    bool JustReleased(const InputComponent& input, unsigned int key) {
        return !input.keydown.test(key) && input.lastKeydown.test(key);
    }

    void PlayJumpSound(EngineView<Components...>* engine) {
        engine->GetSoundManager().GetSound(kSoundJump).play();
    }
};

}  // namespace ECSEngine
