#pragma once

#include "engine/Node.h"

class Asteroid;
class PlayerShip;

// Demo root of the game: owns the ship and asteroid, listens to the ship's
// signals and calls back down into it.
class Level : public engine::Node {
public:
    Level();

private:
    void onShipCrashed(PlayerShip& ship);

    PlayerShip* ship_ = nullptr;
    Asteroid* asteroid_ = nullptr;
};
