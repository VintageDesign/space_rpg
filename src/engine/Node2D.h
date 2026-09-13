#pragma once

#include "engine/Node.h"

namespace engine {

// A node with a position, rotation and scale relative to its parent.
class Node2D : public Node {
public:
    Vec2 position{0.0f, 0.0f};
    float rotation = 0.0f;  // radians, clockwise on screen
    Vec2 scale{1.0f, 1.0f};
    int zIndex = 0;         // higher draws on top
    bool visible = true;

    Transform2D localTransform() const {
        return Transform2D::fromTRS(position, rotation, scale);
    }
    Transform2D globalTransform() const override;

    Vec2 globalPosition() const { return globalTransform().origin; }
    void setGlobalPosition(Vec2 p);

    bool isVisible() const override { return visible; }
};

}  // namespace engine
