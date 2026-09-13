#include "engine/Node.h"

#include "engine/SceneTree.h"

#include <algorithm>
#include <cassert>

namespace engine {

Node::~Node() { disconnectInbound(); }

void Node::trackConnection(std::weak_ptr<detail::ConnectionState> state) {
    inboundConnections_.erase(
        std::remove_if(inboundConnections_.begin(), inboundConnections_.end(),
                       [](const auto& w) { return w.expired(); }),
        inboundConnections_.end());
    inboundConnections_.push_back(std::move(state));
}

void Node::disconnectInbound() {
    for (auto& weak : inboundConnections_) {
        if (auto state = weak.lock()) {
            state->connected = false;
        }
    }
    inboundConnections_.clear();
}

// Runs before destruction so no slot is invoked on a half-destroyed receiver.
void Node::disconnectSubtree() {
    disconnectInbound();
    for (auto& child : children_) {
        child->disconnectSubtree();
    }
}

void Node::queueFree() {
    if (queuedForFree_ || tree_ == nullptr || parent_ == nullptr) {
        return;
    }
    queuedForFree_ = true;
    tree_->pendingFree_.push_back(this);
}

Transform2D Node::globalTransform() const {
    return parent_ ? parent_->globalTransform() : Transform2D{};
}

void Node::adopt(std::unique_ptr<Node> child) {
    assert(child && child->parent_ == nullptr);
    Node* raw = child.get();
    raw->parent_ = this;
    children_.push_back(std::move(child));
    if (tree_ != nullptr) {
        raw->enterTree(tree_);
    }
}

void Node::enterTree(SceneTree* tree) {
    // A child added from inside onEnterTree() has already entered via adopt().
    if (tree_ == tree) {
        return;
    }
    tree_ = tree;
    tree->nodeEntered(this);
    onEnterTree();
    for (size_t i = 0; i < children_.size(); ++i) {
        children_[i]->enterTree(tree);
    }
    if (!readyCalled_) {
        readyCalled_ = true;
        ready();
    }
}

void Node::exitTree() {
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        (*it)->exitTree();
    }
    onExitTree();
    tree_->nodeExited(this);
    tree_ = nullptr;
}

// Stops exit hooks from queueing nodes that are about to be destroyed anyway.
void Node::markSubtreeFreed() {
    queuedForFree_ = true;
    for (auto& child : children_) {
        child->markSubtreeFreed();
    }
}

std::unique_ptr<Node> Node::detachChild(Node* child) {
    auto it = std::find_if(children_.begin(), children_.end(),
                           [child](const auto& c) { return c.get() == child; });
    assert(it != children_.end());
    child->markSubtreeFreed();
    if (child->tree_ != nullptr) {
        child->exitTree();
    }
    child->disconnectSubtree();
    std::unique_ptr<Node> owned = std::move(*it);
    children_.erase(it);
    owned->parent_ = nullptr;
    return owned;
}

}  // namespace engine
