#include "backend/player_ship.h"

#include "engine/SceneTree.h"

#include <algorithm>

using namespace engine;

namespace {

constexpr float kTurnSpeed = 3.5f;   // rad/s
constexpr float kThrust = 400.0f;    // px/s^2
constexpr float kDamping = 0.8f;     // fraction of velocity lost per second
constexpr Color kHullColor = Color::white();

float wrap(float v, float max) {
    return v < 0.0f ? v + max : (v >= max ? v - max : v);
}

}  // namespace

PlayerShip::PlayerShip() {
    name = "PlayerShip";

    hull_ = addChild<Sprite>(ShapeKind::Triangle, Vec2{32, 40}, kHullColor);
    hull_->zIndex = 1;

    flame_ = addChild<Sprite>(ShapeKind::Triangle, Vec2{14, 16},
                              Color{1.0f, 0.6f, 0.1f, 0.9f});
    flame_->position = {0, 28};
    flame_->rotation = kPi;
    flame_->visible = false;

    hitbox_ = addChild<Area2D>(CircleShape{16});
    connect(hitbox_->areaEntered, this, &PlayerShip::onHitboxEntered);
    connect(hitbox_->areaExited, this, &PlayerShip::onHitboxExited);
}

void PlayerShip::respawn(Vec2 at) {
    position = at;
    rotation = 0.0f;
    velocity_ = {};
}

void PlayerShip::onHitboxEntered(Area2D&) {
    hull_->color = Color::red();
    crashed.fire(*this);
}

void PlayerShip::onHitboxExited(Area2D&) {
    if (hitbox_->overlapping().empty()) {
        hull_->color = kHullColor;
    }
}

void PlayerShip::update(float dt) {
    const Input& input = tree()->input;

    if (input.isKeyDown(Qt::Key_Left)) {
        rotation -= kTurnSpeed * dt;
    }
    if (input.isKeyDown(Qt::Key_Right)) {
        rotation += kTurnSpeed * dt;
    }

    const bool thrusting = input.isKeyDown(Qt::Key_Up);
    flame_->visible = thrusting;
    if (thrusting) {
        velocity_ += Vec2{0.0f, -1.0f}.rotated(rotation) * (kThrust * dt);
    }
    velocity_ *= std::max(0.0f, 1.0f - kDamping * dt);
    position += velocity_ * dt;

    const Vec2 view = tree()->viewportSize;
    position = {wrap(position.x, view.x), wrap(position.y, view.y)};
}
