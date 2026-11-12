// Unit tests for the engine pieces that don't need a window: EntityManager and MathUtil.
// No test framework so it builds anywhere (lab machines included). Exit code 0 = all passed.

#include <iostream>
#include <string>
#include <vector>

#include "CollisionComponent.h"
#include "EntityManager.h"
#include "LocationComponent.h"
#include "MathUtil.h"

namespace {

int gChecks = 0;
int gFailures = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++gChecks;                                                                    \
        if (!(cond)) {                                                                \
            ++gFailures;                                                              \
            std::cerr << __FILE__ << ":" << __LINE__ << "  CHECK failed: " #cond "\n"; \
        }                                                                             \
    } while (0)

struct Health {
    int hp = 0;
};
struct Tag {
    char c = 0;
};

using Manager = ECSEngine::EntityManager<Health, Tag, ECSEngine::LocationComponent>;
using ECSEngine::EntityID;

std::vector<EntityID> ValidIDs(Manager& em) {
    std::vector<EntityID> ids;
    for (const auto& e : em) {
        if (e.IsValid()) {
            ids.push_back(e.GetID());
        }
    }
    return ids;
}

void TestCreateGivesSequentialIDs() {
    Manager em;
    CHECK(em.CreateEntity("a") == 0);
    CHECK(em.CreateEntity("b") == 1);
    CHECK(em.CreateEntity("c") == 2);
    CHECK(ValidIDs(em).size() == 3);
}

void TestRemoveLeavesGapAndIteratorSkipsIt() {
    Manager em;
    EntityID a = em.CreateEntity("a");
    EntityID b = em.CreateEntity("b");
    EntityID c = em.CreateEntity("c");
    em.RemoveEntity(b);
    std::vector<EntityID> ids = ValidIDs(em);
    CHECK(ids.size() == 2);
    CHECK(ids[0] == a);
    CHECK(ids[1] == c);
    em.RemoveEntity(b);  // removing twice is a no-op
    CHECK(ValidIDs(em).size() == 2);
}

void TestRemovedIDIsReused() {
    Manager em;
    (void)em.CreateEntity("a");
    EntityID b = em.CreateEntity("b");
    em.RemoveEntity(b);
    CHECK(em.CreateEntity("new") == b);
    CHECK(em.CreateEntity("next") == 2);
}

void TestAddHasGetRemoveComponent() {
    Manager em;
    EntityID e = em.CreateEntity("e");
    CHECK(!em.HasComponent<Health>(e));
    em.AddComponent(e, Health{5});
    CHECK(em.HasComponent<Health>(e));
    CHECK(!em.HasComponent<Tag>(e));
    CHECK(em.GetComponent<Health>(e).hp == 5);
    em.GetComponent<Health>(e).hp = 7;  // reference writes through
    CHECK(em.GetComponent<Health>(e).hp == 7);
    em.RemoveComponent<Health>(e);
    CHECK(!em.HasComponent<Health>(e));
    em.RemoveComponent<Health>(e);  // no-op
    CHECK(!em.HasComponent<Health>(e));
}

void TestAddReplacesExisting() {
    Manager em;
    EntityID e = em.CreateEntity("e");
    em.AddComponent(e, Health{1});
    em.AddComponent(e, Health{2});
    CHECK(em.GetComponent<Health>(e).hp == 2);
    auto& storage = em.GetComponentStorage<Health>();
    CHECK(storage.mStorage.size() == 1);  // replaced, not duplicated
}

void TestRemoveEntityRemovesComponents() {
    Manager em;
    EntityID e = em.CreateEntity("e");
    em.AddComponent(e, Health{3});
    em.AddComponent(e, Tag{'x'});
    em.RemoveEntity(e);
    EntityID reused = em.CreateEntity("reused");
    CHECK(reused == e);
    CHECK(!em.HasComponent<Health>(reused));  // old components must not leak into the new entity
    CHECK(!em.HasComponent<Tag>(reused));
}

void TestStorageReusesFreeSlotAndTracksOwner() {
    Manager em;
    EntityID a = em.CreateEntity("a");
    EntityID b = em.CreateEntity("b");
    em.AddComponent(a, Health{1});
    em.RemoveComponent<Health>(a);
    auto& storage = em.GetComponentStorage<Health>();
    CHECK(!storage.mValid[0]);
    em.AddComponent(b, Health{9});
    CHECK(storage.mStorage.size() == 1);  // slot 0 was reused
    CHECK(storage.mValid[0]);
    CHECK(storage.mOwner[0] == b);
    CHECK(em.GetComponent<Health>(b).hp == 9);
}

void TestComponentsOnLaterEntities() {
    Manager em;
    std::vector<EntityID> ids;
    for (int i = 0; i < 50; ++i) {
        ids.push_back(em.CreateEntity("e"));
    }
    em.AddComponent(ids[49], Tag{'z'});  // storage index map must grow to fit
    CHECK(em.HasComponent<Tag>(ids[49]));
    CHECK(!em.HasComponent<Tag>(ids[10]));
}

void TestRectIntersection() {
    using ECSEngine::Rect;
    Rect a(0, 0, 10, 10);
    CHECK(Intersects(a, Rect(5, 5, 10, 10)));
    CHECK(!Intersects(a, Rect(10, 0, 10, 10)));  // touching edge is not overlap
    CHECK(!Intersects(a, Rect(0, 10, 10, 10)));
    CHECK(!Intersects(a, Rect(20, 20, 5, 5)));
    ECSEngine::Point2D o = Overlap(a, Rect(7, 8, 10, 10));
    CHECK(o.x == 3 && o.y == 2);
}

void TestPointMath() {
    using ECSEngine::Point2D;
    Point2D p = Point2D(1.0f, 2.0f) + Point2D(3.0f, 4.0f);
    CHECK(p == Point2D(4.0f, 6.0f));
    CHECK(p * 0.5f == Point2D(2.0f, 3.0f));
    CHECK(-p == Point2D(-4.0f, -6.0f));
    CHECK(Point2D(3.0f, 4.0f).Length() == 5.0f);
    CHECK(ECSEngine::MoveTowards(0.0f, 10.0f, 3.0f) == 3.0f);
    CHECK(ECSEngine::MoveTowards(9.0f, 10.0f, 3.0f) == 10.0f);
    CHECK(ECSEngine::Rect(0, 0, 4, 4).Translated(Point2D(2.0f, 3.0f)).Right() == 6.0f);
}

void TestCollisionFactories() {
    auto s = ECSEngine::CollisionComponent::Static(ECSEngine::Rect(0, 0, 64, 64), true);
    CHECK(s.staticEntity && s.doesDamage && !s.collectable && !s.onGround);
    auto d = ECSEngine::CollisionComponent::Dynamic(ECSEngine::Rect(0, 0, 40, 40), 0.5f, true);
    CHECK(!d.staticEntity && d.collectable && d.restitution == 0.5f);
}

}  // namespace

int main() {
    TestCreateGivesSequentialIDs();
    TestRemoveLeavesGapAndIteratorSkipsIt();
    TestRemovedIDIsReused();
    TestAddHasGetRemoveComponent();
    TestAddReplacesExisting();
    TestRemoveEntityRemovesComponents();
    TestStorageReusesFreeSlotAndTracksOwner();
    TestComponentsOnLaterEntities();
    TestRectIntersection();
    TestPointMath();
    TestCollisionFactories();

    std::cout << (gChecks - gFailures) << "/" << gChecks << " checks passed\n";
    return gFailures == 0 ? 0 : 1;
}
