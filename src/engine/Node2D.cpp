#include "engine/Node2D.h"

namespace engine {

Transform2D Node2D::globalTransform() const {
    const Transform2D local = localTransform();
    return parent() ? parent()->globalTransform() * local : local;
}

void Node2D::setGlobalPosition(Vec2 p) {
    position = parent() ? parent()->globalTransform().inverse().apply(p) : p;
}

}  // namespace engine
