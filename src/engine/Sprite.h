#pragma once

#include "engine/Node2D.h"

namespace engine {

enum class ShapeKind { Rect, Triangle, Circle };

// Draws a solid colored shape centered on the node. For Circle, size.x is the
// diameter. Textures will slot in here later.
class Sprite : public Node2D {
public:
    Sprite() = default;
    Sprite(ShapeKind kind, Vec2 size, Color color)
        : kind(kind), size(size), color(color) {}

    ShapeKind kind = ShapeKind::Rect;
    Vec2 size{16.0f, 16.0f};
    Color color = Color::white();

protected:
    void draw(RenderQueue& queue) override;
};

}  // namespace engine
