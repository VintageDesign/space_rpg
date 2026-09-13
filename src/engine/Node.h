#pragma once

#include "engine/Connection.h"
#include "engine/Math.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine {

class RenderQueue;
class SceneTree;

// Base of the composition tree. A node owns its children; game objects are
// built by adding child components (Sprite, Area2D, ...) to a parent node.
//
// Lifetime rule: nodes are destroyed only by queueFree(), which is flushed at
// the end of SceneTree::tick(). A pointer returned by addChild() therefore
// stays valid at least until the end of the frame in which queueFree() is
// called on it (or an ancestor).
//
// Communication follows "call down, signal up": parents call methods on the
// children they hold pointers to; children emit Signals (engine/Signal.h)
// that parents connect to.
class Node {
public:
    Node() = default;
    virtual ~Node();

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    template <class T, class... Args>
    T* addChild(Args&&... args) {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = child.get();
        adopt(std::move(child));
        return raw;
    }

    // No-op for nodes outside a tree and for the tree root.
    void queueFree();
    bool isQueuedForFree() const { return queuedForFree_; }

    Node* parent() const { return parent_; }
    SceneTree* tree() const { return tree_; }
    const std::vector<std::unique_ptr<Node>>& children() const {
        return children_;
    }

    // A plain Node passes its parent's transform through unchanged.
    virtual Transform2D globalTransform() const;

    // Hiding a node hides its whole subtree.
    virtual bool isVisible() const { return true; }

    // Used by Signal: connections with this node as receiver are cut when it
    // is freed.
    void trackConnection(std::weak_ptr<detail::ConnectionState> state);

    std::string name;

protected:
    // Called once, after the node and all its children have entered a tree.
    virtual void ready() {}
    virtual void update(float /*dt*/) {}
    virtual void draw(RenderQueue& /*queue*/) {}
    virtual void onEnterTree() {}
    virtual void onExitTree() {}

private:
    friend class SceneTree;

    void adopt(std::unique_ptr<Node> child);
    void enterTree(SceneTree* tree);
    void exitTree();
    void markSubtreeFreed();
    void disconnectInbound();
    void disconnectSubtree();
    std::unique_ptr<Node> detachChild(Node* child);

    Node* parent_ = nullptr;
    SceneTree* tree_ = nullptr;
    std::vector<std::unique_ptr<Node>> children_;
    std::vector<std::weak_ptr<detail::ConnectionState>> inboundConnections_;
    bool readyCalled_ = false;
    bool queuedForFree_ = false;
};

}  // namespace engine
