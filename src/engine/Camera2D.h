#pragma once

#include "engine/Node2D.h"

namespace engine {

// Drives SceneTree::view while it is the tree's current camera. The view is
// centered on the camera's global position, resolved at the end of each tick
// so it sees movement from update(), collision callbacks and deferred calls.
// Parent it to a node to follow that node.
class Camera2D : public Node2D {
public:
    float zoom = 1.0f;            // >1 magnifies
    float visibleHeight = 36.0f;  // world units shown top-to-bottom at zoom 1
    // Off: the view stays upright when an ancestor rotates.
    bool followRotation = false;

    // Safe to call before the camera is in a tree; it takes over on entering.
    void makeCurrent();
    bool isCurrent() const;

protected:
    void onEnterTree() override;

private:
    bool pendingCurrent_ = false;
};

}  // namespace engine
