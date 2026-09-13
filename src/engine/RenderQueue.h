#pragma once

#include "engine/Math.h"

#include <cstdint>
#include <vector>

namespace engine {

// Layout is mirrored by the Vulkan vertex description in src/Vertex.h.
struct Vertex2D {
    float pos[2];
    float color[4];
};

// Backend-agnostic list of world-space triangles for one frame.
class RenderQueue {
public:
    void clear();

    // Rect and triangle are centered on the transform origin; the triangle
    // points toward local -y ("up" at rotation 0).
    void addRect(const Transform2D& t, Vec2 size, Color color, int z);
    void addTriangle(const Transform2D& t, Vec2 size, Color color, int z);
    void addCircle(const Transform2D& t, float radius, Color color, int z,
                   int segments = 24);
    void addTriangleList(const Transform2D& t, const std::vector<Vec2>& points,
                         Color color, int z);

    // Stable-sorts primitives by z (ties keep submission order) into `out`.
    void flatten(std::vector<Vertex2D>& out) const;

    bool empty() const { return vertices_.empty(); }

private:
    struct Primitive {
        int z;
        uint32_t first;
        uint32_t count;
    };

    void push(const Transform2D& t, const Vec2* points, uint32_t count,
              Color color, int z);

    std::vector<Vertex2D> vertices_;
    std::vector<Primitive> primitives_;
};

}  // namespace engine
