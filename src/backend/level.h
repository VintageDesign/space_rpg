#pragma once

#include "engine/Node.h"

class Asteroid;
class PlayerShip;

// Demo root of the game: owns the ship (with a following camera), asteroids
// and a star backdrop; listens to the ship's signals and calls back down.
class Level : public engine::Node {
public:
    Level();

private:
    void scatterBackdrop();
    void onShipCrashed(PlayerShip& ship);

    PlayerShip* ship_ = nullptr;
    Asteroid* asteroid_ = nullptr;
};
