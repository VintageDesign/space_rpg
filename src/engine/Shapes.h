#pragma once

#include "engine/Math.h"

#include <variant>

namespace engine {

// Collision shapes, centered on their owning node's origin.
struct RectShape {
    Vec2 size;
};

struct CircleShape {
    float radius = 0.0f;
};

using Shape = std::variant<RectShape, CircleShape>;

}  // namespace engine
