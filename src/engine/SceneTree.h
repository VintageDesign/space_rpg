#pragma once

#include "engine/Input.h"
#include "engine/Math.h"
#include "engine/Node.h"

#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace engine {

class Area2D;
class RenderQueue;

// Owns the root node and drives a frame, in order: update, collision, deferred
// calls, frees.
class SceneTree {
public:
    SceneTree();
    ~SceneTree();

    SceneTree(const SceneTree&) = delete;
    SceneTree& operator=(const SceneTree&) = delete;

    Node& root() { return *root_; }

    void tick(float dt);
    void buildRenderQueue(RenderQueue& queue);

    // Runs `fn` during this tick's deferred flush, before queued frees, so
    // nodes freed this frame are still alive. Calls queued while frees are
    // flushing (e.g. from onExitTree) run next tick; don't capture other
    // freed nodes in those.
    void callDeferred(std::function<void()> fn);

    Input input;
    // World units are pixels; set by the renderer each frame.
    Vec2 viewportSize{1.0f, 1.0f};

private:
    friend class Node;

    using AreaPair = std::pair<Area2D*, Area2D*>;

    void updateNode(Node& node, float dt);
    void drawNode(Node& node, RenderQueue& queue);
    void physicsStep();
    void flushDeferred();
    void flushFrees();
    void nodeEntered(Node* node);
    void nodeExited(Node* node);

    std::unique_ptr<Node> root_;
    std::vector<Node*> pendingFree_;
    std::vector<std::function<void()>> deferredCalls_;
    std::vector<Area2D*> areas_;
    std::set<AreaPair> overlaps_;
};

}  // namespace engine
