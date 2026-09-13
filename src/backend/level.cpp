#include "backend/level.h"

#include "backend/asteroid.h"
#include "backend/player_ship.h"
#include "engine/Signal.h"

using namespace engine;

namespace {

constexpr Vec2 kSpawnPoint{200, 300};

}  // namespace

Level::Level() {
    name = "Level";

    ship_ = addChild<PlayerShip>();
    ship_->position = kSpawnPoint;

    asteroid_ = addChild<Asteroid>(Vec2{90, 90}, 0.6f);
    asteroid_->position = {500, 300};

    // Deferred: teleporting inside the physics pass's signal loop would change
    // positions mid-step.
    connect(ship_->crashed, this, &Level::onShipCrashed,
            ConnectionType::Deferred);
}

void Level::onShipCrashed(PlayerShip& ship) { ship.respawn(kSpawnPoint); }
