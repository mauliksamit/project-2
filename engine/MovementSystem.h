#pragma once

#include "ECSEngine.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entityManager = engine->GetEntityManager();
        ComponentStorage<MovementComponent>& movementStorage =
            entityManager.template GetComponentStorage<MovementComponent>();

        for (size_t slot = 0; slot < movementStorage.mStorage.size(); slot++) {
            if (!movementStorage.mValid[slot]) {
                continue;
            }

            EntityID owner = movementStorage.mOwner[slot];
            if (!entityManager.template HasComponent<LocationComponent>(owner)) {
                continue;
            }

            Point2D& location = entityManager.template GetComponent<LocationComponent>(owner).loc;
            Point2D velocity = movementStorage.mStorage[slot].velocity;
            location.x += velocity.x;
            location.y += velocity.y;
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
