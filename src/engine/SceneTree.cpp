#include "engine/SceneTree.h"

#include "engine/Area2D.h"
#include "engine/Camera2D.h"
#include "engine/Collision.h"

#include <algorithm>
#include <iterator>

namespace engine {


SceneTree::SceneTree() : root_(std::make_unique<Node>()) {
    root_->name = "root";
    root_->enterTree(this);
}

SceneTree::~SceneTree() {
    root_->markSubtreeFreed();
    root_->exitTree();
    root_->disconnectSubtree();
}

void SceneTree::tick(float dt) {
    updateNode(*root_, dt);
    physicsStep();
    flushDeferred();
    flushFrees();
    updateView();
    input.endFrame();
}

void SceneTree::callDeferred(std::function<void()> fn) {
    deferredCalls_.push_back(std::move(fn));
}

void SceneTree::flushDeferred() {
    // Calls queued by deferred calls run in this same flush.
    while (!deferredCalls_.empty()) {
        std::vector<std::function<void()>> calls = std::move(deferredCalls_);
        deferredCalls_.clear();
        for (auto& call : calls) {
            call();
        }
    }
}

void SceneTree::buildRenderQueue(RenderQueue& queue) {
    drawNode(*root_, queue);
}

void SceneTree::updateNode(Node& node, float dt) {
    if (node.queuedForFree_) {
        return;
    }
    node.update(dt);
    // Indexed: update() may append children, which reallocates the vector.
    for (size_t i = 0; i < node.children_.size(); ++i) {
        updateNode(*node.children_[i], dt);
    }
}

void SceneTree::drawNode(Node& node, RenderQueue& queue) {
    if (!node.isVisible()) {
        return;
    }
    node.draw(queue);
    for (auto& child : node.children_) {
        drawNode(*child, queue);
    }
}

void SceneTree::physicsStep() {
    std::vector<Transform2D> world;
    world.reserve(areas_.size());
    for (Area2D* area : areas_) {
        world.push_back(area->globalTransform());
    }

    std::set<AreaPair> current;
    for (size_t i = 0; i < areas_.size(); ++i) {
        for (size_t j = i + 1; j < areas_.size(); ++j) {
            Area2D* a = areas_[i];
            Area2D* b = areas_[j];
            if (!a->detects(*b) && !b->detects(*a)) {
                continue;
            }
            if (collision::overlaps(a->shape, world[i], b->shape, world[j])) {
                current.insert(std::minmax(a, b));
            }
        }
    }

    std::vector<AreaPair> exited;
    std::vector<AreaPair> entered;
    std::set_difference(overlaps_.begin(), overlaps_.end(), current.begin(),
                        current.end(), std::back_inserter(exited));
    std::set_difference(current.begin(), current.end(), overlaps_.begin(),
                        overlaps_.end(), std::back_inserter(entered));
    overlaps_ = std::move(current);

    // Callbacks may queueFree or add nodes; both are safe here because frees
    // are deferred and new areas only join the next physics step.
    for (auto [a, b] : exited) {
        a->testExited(*b);
        b->testExited(*a);
    }
    for (auto [a, b] : entered) {
        a->testEntered(*b);
        b->testEntered(*a);
    }
}

void SceneTree::updateView() {
    if (currentCamera_ == nullptr) {
        return;
    }
    const Transform2D t = currentCamera_->globalTransform();
    view.center = t.origin;
    view.zoom = currentCamera_->zoom;
    view.visibleHeight = currentCamera_->visibleHeight;
    view.rotation = currentCamera_->followRotation ? t.rotation() : 0.0f;
}

void SceneTree::flushFrees() {
    std::vector<Node*> pending = std::move(pendingFree_);
    pendingFree_.clear();

    // Skip nodes whose ancestor is also being freed; they go with it.
    std::vector<Node*> roots;
    for (Node* node : pending) {
        bool ancestorPending = false;
        for (Node* p = node->parent_; p != nullptr; p = p->parent_) {
            if (p->queuedForFree_) {
                ancestorPending = true;
                break;
            }
        }
        if (!ancestorPending) {
            roots.push_back(node);
        }
    }

    for (Node* node : roots) {
        node->parent_->detachChild(node);
    }
}

void SceneTree::nodeEntered(Node* node) {
    if (auto* area = dynamic_cast<Area2D*>(node)) {
        areas_.push_back(area);
    }
}

void SceneTree::nodeExited(Node* node) {
    if (node == currentCamera_) {
        currentCamera_ = nullptr;
    }
    auto* area = dynamic_cast<Area2D*>(node);
    if (area == nullptr) {
        return;
    }
    areas_.erase(std::remove(areas_.begin(), areas_.end(), area),
                 areas_.end());
    for (auto it = overlaps_.begin(); it != overlaps_.end();) {
        if (it->first == area || it->second == area) {
            it = overlaps_.erase(it);
        } else {
            ++it;
        }
    }
    for (Area2D* other : areas_) {
        auto& list = other->overlapping_;
        list.erase(std::remove(list.begin(), list.end(), area), list.end());
    }
    area->overlapping_.clear();
}

}  // namespace engine
