#include "engine/Camera2D.h"

#include "engine/SceneTree.h"

namespace engine {

void Camera2D::makeCurrent() {
    if (tree() != nullptr) {
        tree()->setCurrentCamera(this);
    } else {
        pendingCurrent_ = true;
    }
}

bool Camera2D::isCurrent() const {
    return tree() != nullptr ? tree()->currentCamera() == this
                             : pendingCurrent_;
}

void Camera2D::onEnterTree() {
    if (pendingCurrent_) {
        pendingCurrent_ = false;
        tree()->setCurrentCamera(this);
    }
}

}  // namespace engine
