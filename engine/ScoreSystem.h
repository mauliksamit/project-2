#pragma once

#include <algorithm>
#include <string>

#include "ScoreComponent.h"
#include "SpriteComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// Points each score digit entity at the sprite for its number.
// Iterates only the ScoreComponent storage and does nothing if the score hasn't changed.
template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& em = engine->GetEntityManager();
        auto& scores = em.template GetComponentStorage<ScoreComponent>();

        for (std::size_t i = 0; i < scores.mStorage.size(); ++i) {
            if (!scores.mValid[i]) {
                continue;
            }
            ScoreComponent& score = scores.mStorage[i];
            if (score.score == score.displayedScore) {
                continue;
            }
            score.displayedScore = score.score;

            int value = std::clamp(score.score, 0, 999);
            // fill digits from the right (least significant) to the left
            for (std::size_t d = ScoreComponent::kNumDigits; d-- > 0;) {
                EntityID digit = score.digitID[d];
                if (em.template HasComponent<SpriteComponent>(digit)) {
                    em.template GetComponent<SpriteComponent>(digit).textureID =
                        score.digitTextureID[static_cast<std::size_t>(value % 10)];
                }
                value /= 10;
            }
        }
        return kContinue;
    }
};

}  // namespace ECSEngine
