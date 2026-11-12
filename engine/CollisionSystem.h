#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "CameraComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "Constants.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// Detects and resolves collisions after MovementSystem has moved everything.
//
// Dynamic vs static: the move from previousCollisionBox to the current box is replayed one
// axis at a time (x first, then y), so an entity sliding down a wall never snags on the seam
// between two wall tiles. Afterwards the contact flags are set by probing 1px around the box.
// Dynamic vs dynamic: the player collects collectables, stars bounce off each other.
//
// Side effects: plays kSoundLand / kSoundStar, adds to ScoreComponent::score, starts a
// CameraShakeComponent shake and removes collected entities (after all loops are done, since
// removing invalidates component references).
template <typename... Components>
class CollisionSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();
        auto& colliders = em.template GetComponentStorage<CollisionComponent>();

        mStatics.clear();
        mDynamics.clear();
        mToRemove.clear();

        // 1. split colliders and clear last frame's contact flags
        for (std::size_t i = 0; i < colliders.mStorage.size(); ++i) {
            if (!colliders.mValid[i] ||
                !em.template HasComponent<LocationComponent>(colliders.mOwner[i])) {
                continue;
            }
            CollisionComponent& c = colliders.mStorage[i];
            if (c.staticEntity) {
                mStatics.push_back(i);
                continue;
            }
            mDynamics.push_back({i, static_cast<bool>(c.onGround),
                                 static_cast<bool>(c.onLeftWall || c.onRightWall)});
            c.onGround = c.onCeiling = c.onLeftWall = c.onRightWall = 0;
        }

        // 2. dynamic vs static
        for (const DynamicInfo& info : mDynamics) {
            ResolveAgainstStatics(engine, info);
        }

        // 3. dynamic vs dynamic
        for (std::size_t a = 0; a < mDynamics.size(); ++a) {
            for (std::size_t b = a + 1; b < mDynamics.size(); ++b) {
                ResolvePair(engine, mDynamics[a].slot, mDynamics[b].slot);
            }
        }

        // 4. remove collected entities now that no references are held
        std::sort(mToRemove.begin(), mToRemove.end());
        mToRemove.erase(std::unique(mToRemove.begin(), mToRemove.end()), mToRemove.end());
        for (EntityID id : mToRemove) {
            em.RemoveEntity(id);
        }
        return kContinue;
    }

private:
    struct DynamicInfo {
        std::size_t slot;  // index into the CollisionComponent storage
        bool wasOnGround;
        bool wasOnWall;
    };

    static constexpr float kShakeMinSpeed = 4.0f;   // slower hits don't shake the camera
    static constexpr float kShakeScale = 0.6f;      // shake px per px/frame of impact
    static constexpr float kShakeMax = 10.0f;
    static constexpr int kShakeFrames = 10;
    static constexpr float kLandSoundMinSpeed = 2.0f;
    static constexpr float kStopSpeed = 0.5f;       // bounces slower than this just stop

    Rect WorldBox(EngineView<Components...>* engine, EntityID id, const CollisionComponent& c) {
        return c.collisionBox.Translated(
            engine->GetEntityManager().template GetComponent<LocationComponent>(id).loc);
    }

    void ResolveAgainstStatics(EngineView<Components...>* engine, const DynamicInfo& info) {
        auto& em = engine->GetEntityManager();
        auto& colliders = em.template GetComponentStorage<CollisionComponent>();
        CollisionComponent& dyn = colliders.mStorage[info.slot];
        EntityID id = colliders.mOwner[info.slot];
        LocationComponent& location = em.template GetComponent<LocationComponent>(id);
        const bool isPlayer = em.template HasComponent<InputComponent>(id);

        Point2D velocity;
        if (em.template HasComponent<MovementComponent>(id)) {
            velocity = em.template GetComponent<MovementComponent>(id).velocity;
        }
        const Point2D impactVelocity = velocity;

        const Rect current = dyn.collisionBox.Translated(location.loc);
        Rect box = dyn.previousCollisionBox;
        const Point2D delta = current.topLeft - box.topLeft;
        bool hitX = false, hitY = false, damaged = false;

        // x axis
        box.topLeft.x += delta.x;
        for (std::size_t s : mStatics) {
            const CollisionComponent& stat = colliders.mStorage[s];
            Rect other = WorldBox(engine, colliders.mOwner[s], stat);
            if (!Intersects(box, other)) {
                continue;
            }
            damaged = damaged || stat.doesDamage;
            if (delta.x > 0) {
                box.topLeft.x = other.Left() - box.width;
            } else if (delta.x < 0) {
                box.topLeft.x = other.Right();
            } else {
                continue;  // started inside it, let the y pass or next frame sort it out
            }
            hitX = true;
        }

        // y axis
        box.topLeft.y += delta.y;
        for (std::size_t s : mStatics) {
            const CollisionComponent& stat = colliders.mStorage[s];
            Rect other = WorldBox(engine, colliders.mOwner[s], stat);
            if (!Intersects(box, other)) {
                continue;
            }
            damaged = damaged || stat.doesDamage;
            if (delta.y > 0) {
                box.topLeft.y = other.Top() - box.height;
            } else if (delta.y < 0) {
                box.topLeft.y = other.Bottom();
            } else {
                continue;
            }
            hitY = true;
        }

        // spikes: back to the respawn point
        if (damaged && isPlayer && em.template HasComponent<RespawnComponent>(id)) {
            location.loc = em.template GetComponent<RespawnComponent>(id).loc;
            if (em.template HasComponent<MovementComponent>(id)) {
                em.template GetComponent<MovementComponent>(id).velocity = Point2D(0.0f, 0.0f);
            }
            return;
        }

        if (hitX) {
            velocity.x = std::fabs(velocity.x) < kStopSpeed ? 0 : -velocity.x * dyn.restitution;
        }
        if (hitY) {
            velocity.y = std::fabs(velocity.y) < kStopSpeed ? 0 : -velocity.y * dyn.restitution;
        }
        location.loc = box.topLeft - dyn.collisionBox.topLeft;
        if (em.template HasComponent<MovementComponent>(id)) {
            em.template GetComponent<MovementComponent>(id).velocity = velocity;
        }

        SetContactFlags(engine, dyn, box);

        if (!isPlayer) {
            return;
        }
        if (dyn.onGround && !info.wasOnGround && impactVelocity.y >= kLandSoundMinSpeed) {
            engine->GetSoundManager().GetSound(kSoundLand).play();
        }
        // shake only on a new contact, not every frame while pressed into a wall
        if (hitY && !info.wasOnGround && std::fabs(impactVelocity.y) >= kShakeMinSpeed) {
            StartShake(engine, Point2D(0.0f, Sign(impactVelocity.y)), std::fabs(impactVelocity.y));
        } else if (hitX && !info.wasOnWall && std::fabs(impactVelocity.x) >= kShakeMinSpeed) {
            StartShake(engine, Point2D(Sign(impactVelocity.x), 0.0f), std::fabs(impactVelocity.x));
        }
    }

    // Probes 1px around the resolved box so the flags stay set while resting or sliding,
    // even on frames where the entity didn't move into the surface.
    void SetContactFlags(EngineView<Components...>* engine, CollisionComponent& dyn,
                         const Rect& box) {
        auto& colliders = engine->GetEntityManager().template GetComponentStorage<CollisionComponent>();
        const Rect below = box.Translated(Point2D(0.0f, 1.0f));
        const Rect above = box.Translated(Point2D(0.0f, -1.0f));
        const Rect left = box.Translated(Point2D(-1.0f, 0.0f));
        const Rect right = box.Translated(Point2D(1.0f, 0.0f));
        for (std::size_t s : mStatics) {
            Rect other = WorldBox(engine, colliders.mOwner[s], colliders.mStorage[s]);
            if (Intersects(below, other)) dyn.onGround = 1;
            if (Intersects(above, other)) dyn.onCeiling = 1;
            if (Intersects(left, other)) dyn.onLeftWall = 1;
            if (Intersects(right, other)) dyn.onRightWall = 1;
        }
    }

    void ResolvePair(EngineView<Components...>* engine, std::size_t slotA, std::size_t slotB) {
        auto& em = engine->GetEntityManager();
        auto& colliders = em.template GetComponentStorage<CollisionComponent>();
        EntityID a = colliders.mOwner[slotA];
        EntityID b = colliders.mOwner[slotB];
        if (IsQueuedForRemoval(a) || IsQueuedForRemoval(b)) {
            return;
        }
        CollisionComponent& ca = colliders.mStorage[slotA];
        CollisionComponent& cb = colliders.mStorage[slotB];
        Rect boxA = WorldBox(engine, a, ca);
        Rect boxB = WorldBox(engine, b, cb);
        if (!Intersects(boxA, boxB)) {
            return;
        }

        const bool aIsPlayer = em.template HasComponent<InputComponent>(a);
        const bool bIsPlayer = em.template HasComponent<InputComponent>(b);
        if (aIsPlayer && cb.collectable) {
            Collect(engine, b);
            return;
        }
        if (bIsPlayer && ca.collectable) {
            Collect(engine, a);
            return;
        }
        if (aIsPlayer || bIsPlayer) {
            return;  // player walks through other dynamic things
        }
        if (!em.template HasComponent<MovementComponent>(a) ||
            !em.template HasComponent<MovementComponent>(b)) {
            return;
        }

        // two stars: push apart on the shallower axis and swap that velocity component.
        // A star already touching a tile on the side it would be pushed towards stays put and
        // the other one moves the whole way, so a star resting on the floor isn't pushed into it.
        Point2D overlap = Overlap(boxA, boxB);
        Point2D& velA = em.template GetComponent<MovementComponent>(a).velocity;
        Point2D& velB = em.template GetComponent<MovementComponent>(b).velocity;
        Point2D& locA = em.template GetComponent<LocationComponent>(a).loc;
        Point2D& locB = em.template GetComponent<LocationComponent>(b).loc;
        if (overlap.x < overlap.y) {
            const float dirA = boxA.Center().x < boxB.Center().x ? -1.0f : 1.0f;  // A moves this way
            const bool aBlocked = dirA < 0 ? ca.onLeftWall : ca.onRightWall;
            const bool bBlocked = dirA < 0 ? cb.onRightWall : cb.onLeftWall;
            Point2D split = SplitPush(overlap.x, aBlocked, bBlocked);
            locA.x += dirA * split.x;
            locB.x -= dirA * split.y;
            std::swap(velA.x, velB.x);
        } else {
            const float dirA = boxA.Center().y < boxB.Center().y ? -1.0f : 1.0f;
            const bool aBlocked = dirA < 0 ? ca.onCeiling : ca.onGround;
            const bool bBlocked = dirA < 0 ? cb.onGround : cb.onCeiling;
            Point2D split = SplitPush(overlap.y, aBlocked, bBlocked);
            locA.y += dirA * split.x;
            locB.y -= dirA * split.y;
            std::swap(velA.y, velB.y);
        }
    }

    // How far A (x) and B (y) move to separate by `overlap`, given which side is blocked by a tile.
    static Point2D SplitPush(float overlap, bool aBlocked, bool bBlocked) {
        if (aBlocked && !bBlocked) return Point2D(0.0f, overlap);
        if (bBlocked && !aBlocked) return Point2D(overlap, 0.0f);
        return Point2D(overlap / 2, overlap / 2);
    }

    void Collect(EngineView<Components...>* engine, EntityID item) {
        mToRemove.push_back(item);
        engine->GetSoundManager().GetSound(kSoundStar).play();
        auto& scores = engine->GetEntityManager().template GetComponentStorage<ScoreComponent>();
        for (std::size_t i = 0; i < scores.mStorage.size(); ++i) {
            if (scores.mValid[i]) {
                scores.mStorage[i].score += 1;
                break;
            }
        }
    }

    // Adds (or resets) a CameraShakeComponent on every camera, so main.cpp doesn't have to add
    // one up front. Only touches the camera/shake storages, so collider references stay valid.
    void StartShake(EngineView<Components...>* engine, const Point2D& direction, float speed) {
        auto& em = engine->GetEntityManager();
        auto& cameras = em.template GetComponentStorage<CameraComponent>();
        const float magnitude = std::min(speed * kShakeScale, kShakeMax);
        for (std::size_t i = 0; i < cameras.mStorage.size(); ++i) {
            if (cameras.mValid[i]) {
                em.AddComponent(cameras.mOwner[i],
                                CameraShakeComponent{kShakeFrames, direction * magnitude});
            }
        }
    }

    bool IsQueuedForRemoval(EntityID id) const {
        return std::find(mToRemove.begin(), mToRemove.end(), id) != mToRemove.end();
    }

    std::vector<std::size_t> mStatics;
    std::vector<DynamicInfo> mDynamics;
    std::vector<EntityID> mToRemove;
};

}  // namespace ECSEngine
