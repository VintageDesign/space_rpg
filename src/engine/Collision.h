#pragma once

#include "engine/Math.h"
#include "engine/Shapes.h"

namespace engine::collision {

struct Circle {
    Vec2 center;
    float radius = 0.0f;
};

// Oriented box: unit axes and half extents along them.
struct OrientedBox {
    Vec2 center;
    Vec2 axisX{1.0f, 0.0f};
    Vec2 axisY{0.0f, 1.0f};
    Vec2 halfExtents;
};

bool overlaps(const Circle& a, const Circle& b);
bool overlaps(const Circle& a, const OrientedBox& b);
bool overlaps(const OrientedBox& a, const OrientedBox& b);

// Places a local shape in world space. Assumes the transform has no skew.
bool overlaps(const Shape& a, const Transform2D& aWorld, const Shape& b,
              const Transform2D& bWorld);

}  // namespace engine::collision
