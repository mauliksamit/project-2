#pragma once

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CollisionComponent.h"
#include "ECSEngine.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

// the camera only moves left and right, its y never changes
template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        EntityManager<Components...>& entityManager = engine->GetEntityManager();
        WindowManager& windowManager = engine->GetWindowManager();
        ComponentStorage<CameraComponent>& cameraStorage =
            entityManager.template GetComponentStorage<CameraComponent>();

        for (size_t slot = 0; slot < cameraStorage.mStorage.size(); slot++) {
            if (!cameraStorage.mValid[slot]) {
                continue;
            }

            EntityID cameraEntity = cameraStorage.mOwner[slot];
            CameraComponent& camera = cameraStorage.mStorage[slot];
            windowManager.SetWorldScale(camera.worldUnitsPerPixel);
            windowManager.SetWorldCenter(camera.position);

            if (entityManager.template HasComponent<CameraFollowerComponent>(cameraEntity)) {
                EntityID followedEntity =
                    entityManager.template GetComponent<CameraFollowerComponent>(cameraEntity)
                        .entityID;
                Follow(entityManager, windowManager, camera, followedEntity);
            }

            // shake only moves what we see, not the camera itself
            Point2D viewCenter = camera.position;
            if (entityManager.template HasComponent<CameraShakeComponent>(cameraEntity)) {
                CameraShakeComponent& shake =
                    entityManager.template GetComponent<CameraShakeComponent>(cameraEntity);
                if (shake.frameLimit > 0) {
                    viewCenter.x += shake.offset.x;
                    viewCenter.y += shake.offset.y;
                    shake.offset.x = -shake.offset.x;
                    shake.offset.y = -shake.offset.y;
                    shake.frameLimit--;
                }
            }
            windowManager.SetWorldCenter(viewCenter);
        }
        return kContinue;
    }

private:
    // the window manager has to be centered on the camera before this is called
    void Follow(EntityManager<Components...>& entityManager, WindowManager& windowManager,
                CameraComponent& camera, EntityID followedEntity) {
        if (!entityManager.template HasComponent<LocationComponent>(followedEntity)) {
            return;
        }
        if (!entityManager.template HasComponent<CollisionComponent>(followedEntity)) {
            return;
        }

        Point2D location =
            entityManager.template GetComponent<LocationComponent>(followedEntity).loc;
        Rect box =
            entityManager.template GetComponent<CollisionComponent>(followedEntity).collisionBox;
        float worldLeft = location.x + box.topLeft.x;
        float worldTop = location.y + box.topLeft.y;
        float screenLeft = windowManager.WorldToWindow(Point2D(worldLeft, worldTop)).x;
        float screenRight = screenLeft + windowManager.WorldToWindow(box.width);

        float hardLeft = camera.hardExclusion.topLeft.x;
        float hardRight = hardLeft + camera.hardExclusion.width;
        float softLeft = camera.softExclusion.topLeft.x;
        float softRight = softLeft + camera.softExclusion.width;

        // screen pixels to move the camera by, positive is right
        float shift = 0;

        // outside the middle 40%, slowly scroll towards it
        if (screenLeft < softLeft) {
            shift = (screenLeft - softLeft) * camera.speed;
        }
        if (screenRight > softRight) {
            shift = (screenRight - softRight) * camera.speed;
        }

        // never let it into the outer 10%, follow it directly there
        if (screenLeft - shift < hardLeft) {
            shift = screenLeft - hardLeft;
        }
        if (screenRight - shift > hardRight) {
            shift = screenRight - hardRight;
        }

        camera.position.x += windowManager.WindowToWorld(shift);
    }
};

}  // namespace ECSEngine
