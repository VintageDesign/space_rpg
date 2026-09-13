#include "engine/Sprite.h"

#include "engine/RenderQueue.h"

namespace engine {

void Sprite::draw(RenderQueue& queue) {
    const Transform2D t = globalTransform();
    switch (kind) {
        case ShapeKind::Rect:
            queue.addRect(t, size, color, zIndex);
            break;
        case ShapeKind::Triangle:
            queue.addTriangle(t, size, color, zIndex);
            break;
        case ShapeKind::Circle:
            queue.addCircle(t, size.x * 0.5f, color, zIndex);
            break;
    }
}

}  // namespace engine
