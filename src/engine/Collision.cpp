#include "engine/Collision.h"

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <variant>

namespace engine::collision {

namespace {

float projectedRadius(const OrientedBox& box, Vec2 axis) {
    return box.halfExtents.x * std::abs(box.axisX.dot(axis)) +
           box.halfExtents.y * std::abs(box.axisY.dot(axis));
}

Circle toWorld(const CircleShape& shape, const Transform2D& t) {
    const float s = std::max(t.x.length(), t.y.length());
    return {t.origin, shape.radius * s};
}

OrientedBox toWorld(const RectShape& shape, const Transform2D& t) {
    const float sx = t.x.length();
    const float sy = t.y.length();
    return {t.origin, t.x.normalized(), t.y.normalized(),
            Vec2{shape.size.x * 0.5f * sx, shape.size.y * 0.5f * sy}};
}

}  // namespace

bool overlaps(const Circle& a, const Circle& b) {
    const float r = a.radius + b.radius;
    return (b.center - a.center).lengthSquared() < r * r;
}

bool overlaps(const Circle& a, const OrientedBox& b) {
    const Vec2 d = a.center - b.center;
    const Vec2 local{d.dot(b.axisX), d.dot(b.axisY)};
    const Vec2 closest{std::clamp(local.x, -b.halfExtents.x, b.halfExtents.x),
                       std::clamp(local.y, -b.halfExtents.y, b.halfExtents.y)};
    return (local - closest).lengthSquared() < a.radius * a.radius;
}

bool overlaps(const OrientedBox& a, const OrientedBox& b) {
    const Vec2 d = b.center - a.center;
    for (Vec2 axis : {a.axisX, a.axisY, b.axisX, b.axisY}) {
        if (std::abs(d.dot(axis)) >=
            projectedRadius(a, axis) + projectedRadius(b, axis)) {
            return false;
        }
    }
    return true;
}

bool overlaps(const Shape& a, const Transform2D& aWorld, const Shape& b,
              const Transform2D& bWorld) {
    return std::visit(
        [&](const auto& sa, const auto& sb) -> bool {
            const auto wa = toWorld(sa, aWorld);
            const auto wb = toWorld(sb, bWorld);
            using A = std::decay_t<decltype(wa)>;
            using B = std::decay_t<decltype(wb)>;
            if constexpr (std::is_same_v<A, OrientedBox> &&
                          std::is_same_v<B, Circle>) {
                return overlaps(wb, wa);
            } else {
                return overlaps(wa, wb);
            }
        },
        a, b);
}

}  // namespace engine::collision
