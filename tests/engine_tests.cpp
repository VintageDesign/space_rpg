#include "engine/Area2D.h"
#include "engine/Camera2D.h"
#include "engine/Collision.h"
#include "engine/Node2D.h"
#include "engine/RenderQueue.h"
#include "engine/SceneTree.h"
#include "engine/Signal.h"
#include "engine/Sprite.h"
#include "engine/View2D.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

using namespace engine;

namespace {

int failures = 0;

#define CHECK(cond)                                                   \
    do {                                                              \
        if (!(cond)) {                                                \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, \
                         __LINE__, #cond);                            \
            ++failures;                                               \
        }                                                             \
    } while (0)

bool near(Vec2 a, Vec2 b, float eps = 1e-3f) {
    return std::abs(a.x - b.x) < eps && std::abs(a.y - b.y) < eps;
}

// Node whose update() runs an arbitrary callback.
class Scripted : public Node2D {
public:
    std::function<void(Scripted&)> onUpdate;
    int updates = 0;

protected:
    void update(float) override {
        ++updates;
        if (onUpdate) {
            onUpdate(*this);
        }
    }
};

void testTransformComposition() {
    SceneTree tree;
    auto* parent = tree.root().addChild<Node2D>();
    parent->position = {100, 100};
    parent->rotation = kPi / 2;
    auto* child = parent->addChild<Node2D>();
    child->position = {10, 0};
    CHECK(near(child->globalPosition(), {100, 110}));

    parent->scale = {2, 2};
    CHECK(near(child->globalPosition(), {100, 120}));

    child->setGlobalPosition({50, 50});
    CHECK(near(child->globalPosition(), {50, 50}));

    // A plain Node in the chain passes its parent's transform through.
    auto* group = parent->addChild<Node>();
    auto* grandchild = group->addChild<Node2D>();
    CHECK(near(grandchild->globalPosition(), {100, 100}));
}

void testLifecycle() {
    SceneTree tree;
    auto* a = tree.root().addChild<Scripted>();
    auto* b = tree.root().addChild<Scripted>();
    Scripted* spawned = nullptr;

    a->onUpdate = [&](Scripted& self) {
        self.queueFree();
        self.queueFree();  // double free request is harmless
        if (!spawned) {
            spawned = tree.root().addChild<Scripted>();
        }
    };
    b->addChild<Scripted>()->queueFree();
    b->queueFree();  // parent and child both queued

    tree.tick(0.016f);
    CHECK(tree.root().children().size() == 1);
    CHECK(tree.root().children()[0].get() == spawned);
    // Added during update, after the loop reached it by index: updated once.
    CHECK(spawned->updates == 1);

    tree.tick(0.016f);
    CHECK(spawned->updates == 2);
}

void testCollisionShapes() {
    using namespace collision;
    CHECK(overlaps(Circle{{0, 0}, 5}, Circle{{9, 0}, 5}));
    CHECK(!overlaps(Circle{{0, 0}, 5}, Circle{{11, 0}, 5}));

    OrientedBox box{{0, 0}, {1, 0}, {0, 1}, {5, 5}};
    CHECK(overlaps(Circle{{8, 0}, 4}, box));
    CHECK(!overlaps(Circle{{8, 8}, 4}, box));  // near the corner, but outside

    // 45-degree box whose AABB would overlap `box`, but which does not.
    const float s = std::sqrt(0.5f);
    OrientedBox diamond{{12, 12}, {s, s}, {-s, s}, {5, 5}};
    CHECK(!overlaps(box, diamond));
    diamond.center = {8, 8};
    CHECK(overlaps(box, diamond));

    // Via world transforms: rect rotated 45 degrees and scaled.
    const Shape rect = RectShape{{10, 10}};
    CHECK(!overlaps(rect, Transform2D{}, rect,
                    Transform2D::fromTRS({12, 12}, kPi / 4, {1, 1})));
    CHECK(overlaps(rect, Transform2D{}, rect,
                   Transform2D::fromTRS({12, 12}, kPi / 4, {2, 2})));
}

void testAreaEvents() {
    SceneTree tree;
    auto* a = tree.root().addChild<Area2D>(CircleShape{5});
    auto* b = tree.root().addChild<Area2D>(CircleShape{5});
    a->position = {0, 0};
    b->position = {100, 0};

    int entered = 0;
    int exited = 0;
    a->areaEntered.connect([&](Area2D& other) {
        CHECK(&other == b);
        ++entered;
    });
    a->areaExited.connect([&](Area2D&) { ++exited; });

    tree.tick(0.016f);
    CHECK(entered == 0);

    b->position = {6, 0};
    tree.tick(0.016f);
    tree.tick(0.016f);
    CHECK(entered == 1);
    CHECK(a->overlapping().size() == 1);

    b->position = {100, 0};
    tree.tick(0.016f);
    tree.tick(0.016f);
    CHECK(exited == 1);
    CHECK(a->overlapping().empty());

    // Freeing an overlapping area drops it without an exit event.
    b->position = {6, 0};
    tree.tick(0.016f);
    CHECK(entered == 2);
    b->queueFree();
    tree.tick(0.016f);
    CHECK(exited == 1);
    CHECK(a->overlapping().empty());
}

void testLayerMask() {
    SceneTree tree;
    auto* bullet = tree.root().addChild<Area2D>(CircleShape{5});
    auto* enemy = tree.root().addChild<Area2D>(CircleShape{5});
    bullet->layer = 0b01;
    bullet->mask = 0b10;
    enemy->layer = 0b10;
    enemy->mask = 0b00;

    int bulletHits = 0;
    int enemyHits = 0;
    bullet->areaEntered.connect([&](Area2D&) { ++bulletHits; });
    enemy->areaEntered.connect([&](Area2D&) { ++enemyHits; });

    tree.tick(0.016f);
    CHECK(bulletHits == 1);
    CHECK(enemyHits == 0);

    auto* ghost = tree.root().addChild<Area2D>(CircleShape{5});
    ghost->layer = 0b100;
    ghost->mask = 0b100;
    tree.tick(0.016f);
    CHECK(bulletHits == 1);
    CHECK(ghost->overlapping().empty());
}

void testRenderQueueOrdering() {
    SceneTree tree;
    auto* top = tree.root().addChild<Sprite>(ShapeKind::Rect, Vec2{2, 2},
                                             Color{1, 0, 0, 1});
    top->zIndex = 5;
    tree.root().addChild<Sprite>(ShapeKind::Triangle, Vec2{2, 2},
                                 Color{0, 1, 0, 1});
    auto* hidden = tree.root().addChild<Node2D>();
    hidden->visible = false;
    hidden->addChild<Sprite>();

    RenderQueue queue;
    tree.buildRenderQueue(queue);
    std::vector<Vertex2D> verts;
    queue.flatten(verts);
    CHECK(verts.size() == 9);          // triangle + rect; hidden subtree skipped
    CHECK(verts.front().color[1] == 1);  // lower z first
    CHECK(verts.back().color[0] == 1);
}

class Emitter : public Node2D {
public:
    Signal<int> value;
    Signal<Emitter&> self;
    Signal<const std::string&> text;
};

class Listener : public Node2D {
public:
    std::vector<int> values;
    int selfCalls = 0;
    void onValue(int v) { values.push_back(v); }
    void onSelf(Emitter& e) {
        CHECK(e.name == "emitter");
        ++selfCalls;
    }
};

void testSignalsImmediate() {
    SceneTree tree;
    auto* emitter = tree.root().addChild<Emitter>();
    auto* listener = tree.root().addChild<Listener>();

    std::vector<int> order;
    emitter->value.connect([&](int v) { order.push_back(v * 10); });
    connect(emitter->value, listener, &Listener::onValue);
    emitter->value.fire(3);
    CHECK(order.size() == 1 && order[0] == 30);
    CHECK(listener->values.size() == 1 && listener->values[0] == 3);

    Signal<int> sig;
    int a = 0, b = 0, c = 0, late = 0;
    Connection cb;
    Connection ca = sig.connect([&](int) {
        ++a;
        ca.disconnect();  // self
        cb.disconnect();  // a later slot
        sig.connect([&](int) { ++late; });
    });
    cb = sig.connect([&](int) { ++b; });
    Connection cc = sig.connect([&](int) { ++c; });
    sig.fire(1);
    CHECK(a == 1 && b == 0 && c == 1 && late == 0);
    CHECK(!ca.connected() && !cb.connected() && cc.connected());
    sig.fire(1);
    CHECK(a == 1 && c == 2 && late == 1);
    CHECK(sig.connectionCount() == 2);

    cc.disconnect();
    sig.fire(1);
    CHECK(c == 2);

    // Nested emit of the same signal.
    Signal<int> nested;
    int depthCalls = 0;
    Connection once = nested.connect([&](int depth) {
        ++depthCalls;
        if (depth < 3) {
            nested.fire(depth + 1);
        }
        if (depth == 3) {
            once.disconnect();
        }
    });
    nested.fire(0);
    CHECK(depthCalls == 4);
    CHECK(nested.connectionCount() == 0);
}

void testSignalLifetime() {
    SceneTree tree;
    auto* emitter = tree.root().addChild<Emitter>();
    auto* listener = tree.root().addChild<Listener>();
    Connection conn = connect(emitter->value, listener, &Listener::onValue);

    listener->queueFree();
    emitter->value.fire(1);  // still alive this frame
    tree.tick(0.016f);
    CHECK(!conn.connected());
    emitter->value.fire(2);  // listener destroyed: must not be called
    CHECK(emitter->value.connectionCount() == 0);

    // Sender freed: handle reports disconnected.
    auto* listener2 = tree.root().addChild<Listener>();
    Connection fromDead = connect(emitter->value, listener2, &Listener::onValue);
    emitter->queueFree();
    tree.tick(0.016f);
    CHECK(!fromDead.connected());

    // Receiver freed as a descendant of a freed parent.
    auto* emitter2 = tree.root().addChild<Emitter>();
    auto* parent = tree.root().addChild<Node2D>();
    auto* child = parent->addChild<Listener>();
    Connection childConn = connect(emitter2->value, child, &Listener::onValue);
    parent->queueFree();
    tree.tick(0.016f);
    CHECK(!childConn.connected());
    emitter2->value.fire(5);
    CHECK(emitter2->value.connectionCount() == 0);
}

void testSignalsDeferred() {
    SceneTree tree;
    auto* emitter = tree.root().addChild<Emitter>();
    emitter->name = "emitter";
    auto* listener = tree.root().addChild<Listener>();

    connect(emitter->value, listener, &Listener::onValue,
            ConnectionType::Deferred);
    emitter->value.fire(7);
    CHECK(listener->values.empty());
    tree.tick(0.016f);
    CHECK(listener->values.size() == 1 && listener->values[0] == 7);
    tree.tick(0.016f);
    CHECK(listener->values.size() == 1);

    // Sender frees itself in the same frame; the reference is still valid.
    connect(emitter->self, listener, &Listener::onSelf,
            ConnectionType::Deferred);
    auto* script = tree.root().addChild<Scripted>();
    script->onUpdate = [&](Scripted& s) {
        if (s.updates == 1) {
            emitter->self.fire(*emitter);
            emitter->queueFree();
        }
    };
    tree.tick(0.016f);
    CHECK(listener->selfCalls == 1);

    // Reference-to-temporary argument is copied.
    std::string received;
    auto* emitter2 = tree.root().addChild<Emitter>();
    emitter2->text.connect(
        listener, [&](const std::string& s) { received = s; },
        ConnectionType::Deferred);
    emitter2->text.fire(std::string("hello") + " world");
    tree.tick(0.016f);
    CHECK(received == "hello world");

    // Disconnected before the flush: dropped.
    Connection dropped = emitter2->value.connect(
        listener, [&](int v) { listener->onValue(v); },
        ConnectionType::Deferred);
    emitter2->value.fire(99);
    dropped.disconnect();
    tree.tick(0.016f);
    CHECK(listener->values.size() == 1);

    // Deferred emitting deferred, plus callDeferred, all in one tick.
    int chained = 0;
    emitter2->value.connect(
        listener,
        [&](int v) {
            ++chained;
            if (v == 1) {
                emitter2->value.fire(2);
            }
        },
        ConnectionType::Deferred);
    bool direct = false;
    tree.callDeferred([&] { direct = true; });
    emitter2->value.fire(1);
    tree.tick(0.016f);
    CHECK(chained == 2);
    CHECK(direct);
}

void testViewTransform() {
    View2D view;
    view.center = {10, -5};
    view.visibleHeight = 20;
    view.screenSize = {800, 400};

    // Visible height is fixed; width follows the aspect ratio.
    CHECK(near(view.visibleSize(), {40, 20}));
    view.screenSize = {400, 400};
    CHECK(near(view.visibleSize(), {20, 20}));
    view.screenSize = {800, 400};

    const Transform2D clip = view.worldToClip();
    CHECK(near(clip.apply(view.center), {0, 0}));
    CHECK(near(clip.apply(view.center + view.visibleSize() * 0.5f), {1, 1}));
    CHECK(near(clip.apply(view.center - view.visibleSize() * 0.5f), {-1, -1}));

    // Screen space: top-left pixel origin, y-down.
    CHECK(near(view.worldToScreen().apply(view.center), {400, 200}));
    CHECK(near(view.worldToScreen().apply({-10, -15}), {0, 0}));

    view.zoom = 2;
    CHECK(near(view.visibleSize(), {20, 10}));

    view.rotation = 0.7f;
    const Vec2 p{13, 2};
    CHECK(near(view.screenToWorld().apply(view.worldToScreen().apply(p)), p));
    CHECK(near(view.worldToClip().apply(view.center), {0, 0}));
}

void testCamera() {
    SceneTree tree;
    auto* target = tree.root().addChild<Scripted>();
    target->onUpdate = [](Scripted& self) {
        self.position += Vec2{1, 0};
        self.rotation += 0.5f;
    };

    // makeCurrent() before entering the tree takes effect on entry.
    auto* holder = new Node2D;
    auto* camera = holder->addChild<Camera2D>();
    camera->makeCurrent();
    CHECK(camera->isCurrent());
    delete holder;

    camera = target->addChild<Camera2D>();
    camera->position = {0, 2};  // offset from the target, in target space
    camera->zoom = 2;
    camera->makeCurrent();
    CHECK(tree.currentCamera() == camera);

    // The view follows the camera's position but stays upright.
    tree.tick(0.016f);
    CHECK(near(tree.view.center, camera->globalPosition()));
    CHECK(tree.view.rotation == 0.0f);
    CHECK(tree.view.zoom == 2.0f);

    camera->followRotation = true;
    tree.tick(0.016f);
    CHECK(std::abs(tree.view.rotation - 1.0f) < 1e-3f);
    camera->followRotation = false;

    // A deferred teleport lands in the view in the same tick.
    tree.callDeferred([&] { target->position = {50, 50}; });
    tree.tick(0.016f);
    CHECK(near(tree.view.center, camera->globalPosition()));

    // Switching cameras.
    auto* fixed = tree.root().addChild<Camera2D>();
    fixed->position = {-7, 3};
    fixed->makeCurrent();
    CHECK(!camera->isCurrent());
    tree.tick(0.016f);
    CHECK(near(tree.view.center, {-7, 3}));

    // Freeing the current camera clears it; the view keeps its last values.
    fixed->queueFree();
    tree.tick(0.016f);
    CHECK(tree.currentCamera() == nullptr);
    CHECK(near(tree.view.center, {-7, 3}));
}

}  // namespace

int main() {
    testTransformComposition();
    testViewTransform();
    testCamera();
    testLifecycle();
    testCollisionShapes();
    testAreaEvents();
    testLayerMask();
    testRenderQueueOrdering();
    testSignalsImmediate();
    testSignalLifetime();
    testSignalsDeferred();

    if (failures == 0) {
        std::printf("engine_tests: all passed\n");
    }
    return failures == 0 ? 0 : 1;
}
