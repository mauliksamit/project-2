#pragma once

#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "GameObjects.h"
#include "LocationComponent.h"
#include "SpawnComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// Counts every spawner down and creates its entity when the timer runs out.
// New entities are created after the loop, because adding components can reallocate
// the SpawnComponent storage we're iterating.
template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    SpawnSystem() : mRng(std::random_device{}()) {}

    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();
        auto& spawners = em.template GetComponentStorage<SpawnComponent>();
        std::uniform_real_distribution<float> angle(0.0f, 2.0f * kPi);
        mPending.clear();

        for (std::size_t i = 0; i < spawners.mStorage.size(); ++i) {
            if (!spawners.mValid[i]) {
                continue;
            }
            SpawnComponent& spawn = spawners.mStorage[i];
            if (spawn.totalSpawns <= 0 || --spawn.timeToNextSpawn > 0) {
                continue;
            }
            spawn.timeToNextSpawn = spawn.timeBetweenSpawns;
            spawn.totalSpawns -= 1;

            if (!em.template HasComponent<LocationComponent>(spawn.entityID)) {
                continue;
            }
            // centre the star on the spawner tile
            Point2D pos = em.template GetComponent<LocationComponent>(spawn.entityID).loc +
                          Point2D((64.0f - kStarSize) / 2, (64.0f - kStarSize) / 2);
            float a = angle(mRng);
            Point2D velocity(std::cos(a) * spawn.spawnSpeed, std::sin(a) * spawn.spawnSpeed);
            mPending.push_back({spawn.spawnType, pos, velocity, spawn.spawnTextureID});
        }

        for (const PendingSpawn& p : mPending) {
            if (p.type == "star") {
                CreateStar(engine, p.pos, p.velocity, p.sprite);
            }
        }
        return kContinue;
    }

private:
    struct PendingSpawn {
        std::string type;
        Point2D pos;
        Point2D velocity;
        SpriteID sprite;
    };

    std::mt19937 mRng;
    std::vector<PendingSpawn> mPending;
};

}  // namespace ECSEngine
