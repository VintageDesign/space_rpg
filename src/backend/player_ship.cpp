#include "backend/player_ship.h"

#include "engine/SceneTree.h"

#include <algorithm>

using namespace engine;

namespace {

constexpr float kTurnSpeed = 3.5f;   // rad/s
constexpr float kThrust = 20.0f;     // m/s^2
constexpr float kDamping = 0.8f;     // fraction of velocity lost per second
constexpr Color kHullColor = Color::white();

}  // namespace

PlayerShip::PlayerShip() {
    name = "PlayerShip";

    hull_ = addChild<Sprite>(ShapeKind::Triangle, Vec2{1.6f, 2.0f}, kHullColor);
    hull_->zIndex = 1;

    flame_ = addChild<Sprite>(ShapeKind::Triangle, Vec2{0.7f, 0.8f},
                              Color{1.0f, 0.6f, 0.1f, 0.9f});
    flame_->position = {0, 1.4f};
    flame_->rotation = kPi;
    flame_->visible = false;

    camera_ = addChild<Camera2D>();
    camera_->makeCurrent();

    hitbox_ = addChild<Area2D>(CircleShape{0.8f});
    connect(hitbox_->areaEntered, this, &PlayerShip::onHitboxEntered);
    connect(hitbox_->areaExited, this, &PlayerShip::onHitboxExited);
}

void PlayerShip::respawn(Vec2 at) {
    position = at;
    rotation = 0.0f;
    velocity_ = {};
}

void PlayerShip::onHitboxEntered(Area2D& object) {
    if(object.parent()->name == "Salvage")
    {

      object.parent()->queueFree();
    }
    else{
      hull_->color = Color::red();
      crashed.fire(*this);
    }
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
    // Playing with using a frictionless plane.. Like in real life.
    //velocity_ *= std::max(0.0f, 1.0f - kDamping * dt);
    position += velocity_ * dt;
}
