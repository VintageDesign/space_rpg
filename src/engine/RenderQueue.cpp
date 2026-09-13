#include "engine/RenderQueue.h"

#include <algorithm>

namespace engine {

void RenderQueue::clear() {
    vertices_.clear();
    primitives_.clear();
}

void RenderQueue::push(const Transform2D& t, const Vec2* points,
                       uint32_t count, Color color, int z) {
    primitives_.push_back(
        {z, static_cast<uint32_t>(vertices_.size()), count});
    for (uint32_t i = 0; i < count; ++i) {
        const Vec2 p = t.apply(points[i]);
        vertices_.push_back({{p.x, p.y}, {color.r, color.g, color.b, color.a}});
    }
}

void RenderQueue::addRect(const Transform2D& t, Vec2 size, Color color,
                          int z) {
    const Vec2 h = size * 0.5f;
    const Vec2 pts[6] = {{-h.x, -h.y}, {h.x, -h.y}, {h.x, h.y},
                         {-h.x, -h.y}, {h.x, h.y},  {-h.x, h.y}};
    push(t, pts, 6, color, z);
}

void RenderQueue::addTriangle(const Transform2D& t, Vec2 size, Color color,
                              int z) {
    const Vec2 h = size * 0.5f;
    const Vec2 pts[3] = {{0.0f, -h.y}, {h.x, h.y}, {-h.x, h.y}};
    push(t, pts, 3, color, z);
}

void RenderQueue::addCircle(const Transform2D& t, float radius, Color color,
                            int z, int segments) {
    std::vector<Vec2> pts;
    pts.reserve(static_cast<size_t>(segments) * 3);
    const float step = 2.0f * kPi / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i) {
        const float a0 = step * static_cast<float>(i);
        const float a1 = a0 + step;
        pts.push_back({0.0f, 0.0f});
        pts.push_back(Vec2{std::cos(a0), std::sin(a0)} * radius);
        pts.push_back(Vec2{std::cos(a1), std::sin(a1)} * radius);
    }
    addTriangleList(t, pts, color, z);
}

void RenderQueue::addTriangleList(const Transform2D& t,
                                  const std::vector<Vec2>& points, Color color,
                                  int z) {
    push(t, points.data(), static_cast<uint32_t>(points.size()), color, z);
}

void RenderQueue::flatten(std::vector<Vertex2D>& out) const {
    std::vector<const Primitive*> order;
    order.reserve(primitives_.size());
    for (const Primitive& p : primitives_) {
        order.push_back(&p);
    }
    std::stable_sort(order.begin(), order.end(),
                     [](const Primitive* a, const Primitive* b) {
                         return a->z < b->z;
                     });

    out.clear();
    out.reserve(vertices_.size());
    for (const Primitive* p : order) {
        out.insert(out.end(), vertices_.begin() + p->first,
                   vertices_.begin() + p->first + p->count);
    }
}

}  // namespace engine
