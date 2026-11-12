#pragma once

#include <string>

#include "CollisionComponent.h"
#include "Entity.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// Runs first each frame. Saves every collider's world-space box before anything moves,
// so CollisionSystem can tell which side an entity came from.
// Only iterates the CollisionComponent storage.
template <typename... Components>
class CollisionLateSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionLateSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();
        auto& colliders = em.template GetComponentStorage<CollisionComponent>();

        for (std::size_t i = 0; i < colliders.mStorage.size(); ++i) {
            if (!colliders.mValid[i]) {
                continue;
            }
            CollisionComponent& collider = colliders.mStorage[i];
            EntityID owner = colliders.mOwner[i];
            if (collider.staticEntity || !em.template HasComponent<LocationComponent>(owner)) {
                continue;  // statics never move, so their previous box is never needed
            }
            const Point2D& loc = em.template GetComponent<LocationComponent>(owner).loc;
            collider.previousCollisionBox = collider.collisionBox.Translated(loc);
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
