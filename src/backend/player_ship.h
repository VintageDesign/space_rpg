#pragma once

#include "engine/Area2D.h"
#include "engine/Camera2D.h"
#include "engine/Node2D.h"
#include "engine/Signal.h"
#include "engine/Sprite.h"

// Demo object: a hull and engine-flame sprite plus a circular hitbox, all
// positioned relative to the ship.
class PlayerShip : public engine::Node2D {
public:
    PlayerShip();

    engine::Signal<PlayerShip&> crashed;

    void respawn(engine::Vec2 at);

protected:
    void update(float dt) override;

private:
    void onHitboxEntered(engine::Area2D& other);
    void onHitboxExited(engine::Area2D& other);

    engine::Sprite* hull_ = nullptr;
    engine::Sprite* flame_ = nullptr;
    engine::Area2D* hitbox_ = nullptr;
    engine::Camera2D* camera_ = nullptr;
    engine::Vec2 velocity_;
};
