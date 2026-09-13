#pragma once

#include "engine/Input.h"
#include "engine/Math.h"
#include "engine/Node.h"
#include "engine/View2D.h"

#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace engine {

class Area2D;
class Camera2D;
class RenderQueue;

// Owns the root node and drives a frame, in order: update, collision, deferred
// calls, frees, then the view update from the current camera.
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

    // The camera that drives `view`, or null. Cleared when that camera leaves
    // the tree; `view` then keeps its last values.
    void setCurrentCamera(Camera2D* camera) { currentCamera_ = camera; }
    Camera2D* currentCamera() const { return currentCamera_; }

    Input input;
    // World-to-screen mapping. The renderer sets view.screenSize each frame.
    View2D view;

private:
    friend class Node;

    using AreaPair = std::pair<Area2D*, Area2D*>;

    void updateNode(Node& node, float dt);
    void drawNode(Node& node, RenderQueue& queue);
    void physicsStep();
    void flushDeferred();
    void flushFrees();
    void updateView();
    void nodeEntered(Node* node);
    void nodeExited(Node* node);

    std::unique_ptr<Node> root_;
    std::vector<Node*> pendingFree_;
    std::vector<std::function<void()>> deferredCalls_;
    std::vector<Area2D*> areas_;
    std::set<AreaPair> overlaps_;
    Camera2D* currentCamera_ = nullptr;
};

}  // namespace engine
