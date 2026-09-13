#pragma once

#include <algorithm>
#include <cmath>

namespace engine {

inline constexpr float kPi = 3.14159265358979f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    constexpr float dot(Vec2 o) const { return x * o.x + y * o.y; }
    constexpr float lengthSquared() const { return dot(*this); }
    float length() const { return std::sqrt(lengthSquared()); }
    Vec2 normalized() const {
        const float len = length();
        return len > 0.0f ? *this / len : Vec2{};
    }

    // With +y down, a positive angle turns clockwise on screen.
    Vec2 rotated(float radians) const {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        return {c * x - s * y, s * x + c * y};
    }
};

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    static constexpr Color white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color red() { return {1.0f, 0.2f, 0.2f, 1.0f}; }
    static constexpr Color blue() { return {0.2f, 0.2f, 1.0f, 1.0f}; }
    static constexpr Color gray() { return {0.5f, 0.5f, 0.5f, 1.0f}; }
};

// 2x3 affine transform: columns `x` and `y` are the basis vectors, `origin` is
// the translation. apply(p) = x * p.x + y * p.y + origin.
struct Transform2D {
    Vec2 x{1.0f, 0.0f};
    Vec2 y{0.0f, 1.0f};
    Vec2 origin{0.0f, 0.0f};

    static Transform2D fromTRS(Vec2 translation, float rotation, Vec2 scale) {
        const float c = std::cos(rotation);
        const float s = std::sin(rotation);
        Transform2D t;
        t.x = Vec2{c, s} * scale.x;
        t.y = Vec2{-s, c} * scale.y;
        t.origin = translation;
        return t;
    }

    Vec2 applyBasis(Vec2 v) const { return x * v.x + y * v.y; }
    Vec2 apply(Vec2 p) const { return applyBasis(p) + origin; }

    // (a * b).apply(p) == a.apply(b.apply(p))
    Transform2D operator*(const Transform2D& b) const {
        Transform2D t;
        t.x = applyBasis(b.x);
        t.y = applyBasis(b.y);
        t.origin = apply(b.origin);
        return t;
    }

    Transform2D inverse() const {
        const float det = x.x * y.y - y.x * x.y;
        Transform2D t;
        t.x = Vec2{y.y, -x.y} / det;
        t.y = Vec2{-y.x, x.x} / det;
        t.origin = -t.applyBasis(origin);
        return t;
    }

    float rotation() const { return std::atan2(x.y, x.x); }
};

}  // namespace engine
