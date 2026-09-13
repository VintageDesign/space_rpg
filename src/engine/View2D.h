#pragma once

#include "engine/Math.h"

namespace engine {

// Maps world space to the screen. The visible world height is fixed, so a
// wider window shows more world horizontally but never scales it vertically,
// and pixel density has no effect. +y is down in world, clip and screen space.
struct View2D {
    Vec2 center{0.0f, 0.0f};     // world point at the middle of the screen
    float visibleHeight = 36.0f; // world units shown top-to-bottom at zoom 1
    float zoom = 1.0f;           // >1 magnifies
    float rotation = 0.0f;       // radians, clockwise on screen
    Vec2 screenSize{1.0f, 1.0f}; // pixels; set by the renderer

    Vec2 visibleSize() const {
        const float aspect =
            screenSize.y > 0.0f ? screenSize.x / screenSize.y : 1.0f;
        return Vec2{visibleHeight * aspect, visibleHeight} / zoom;
    }

    // World -> [-1, 1] on both axes.
    Transform2D worldToClip() const {
        const Vec2 visible = visibleSize();
        const Transform2D scaleToClip = Transform2D::fromTRS(
            {}, 0.0f, {2.0f / visible.x, 2.0f / visible.y});
        const Transform2D unrotate =
            Transform2D::fromTRS({}, -rotation, {1.0f, 1.0f});
        const Transform2D uncenter =
            Transform2D::fromTRS(-center, 0.0f, {1.0f, 1.0f});
        return scaleToClip * unrotate * uncenter;
    }

    // World -> pixels, origin at the top-left corner.
    Transform2D worldToScreen() const {
        const Vec2 half = screenSize * 0.5f;
        return Transform2D::fromTRS(half, 0.0f, half) * worldToClip();
    }

    Transform2D screenToWorld() const { return worldToScreen().inverse(); }
};

}  // namespace engine
