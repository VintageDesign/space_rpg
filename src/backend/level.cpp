#include "backend/level.h"

#include "backend/asteroid.h"
#include "backend/player_ship.h"
#include "engine/Signal.h"
#include "engine/Sprite.h"

#include <random>

using namespace engine;

namespace {

constexpr Vec2 kSpawnPoint{0, 0};

// Backdrop, scattered over a square centered on the spawn point. A fixed seed
// keeps the layout identical between runs.
constexpr unsigned kBackdropSeed = 1337;
constexpr float kBackdropHalfExtent = 150.0f;  // m
constexpr int kStarCount = 300;
constexpr int kExtraAsteroidCount = 8;
constexpr float kSpawnClearRadius = 12.0f;  // m kept free of asteroids

}  // namespace

Level::Level() {
    name = "Level";

    ship_ = addChild<PlayerShip>();
    ship_->position = kSpawnPoint;

    asteroid_ = addChild<Asteroid>(Vec2{4.5f, 4.5f}, 0.6f);
    asteroid_->position = {15, 0};

    scatterBackdrop();

    // Deferred: teleporting inside the physics pass's signal loop would change
    // positions mid-step.
    connect(ship_->crashed, this, &Level::onShipCrashed,
            ConnectionType::Deferred);
}

void Level::scatterBackdrop() {
    std::mt19937 rng(kBackdropSeed);
    std::uniform_real_distribution<float> coord(-kBackdropHalfExtent,
                                                kBackdropHalfExtent);

    std::uniform_real_distribution<float> starSize(0.1f, 0.25f);
    std::uniform_real_distribution<float> starAlpha(0.3f, 0.9f);
    for (int i = 0; i < kStarCount; ++i) {
        const float size = starSize(rng);
        auto* star = addChild<Sprite>(ShapeKind::Rect, Vec2{size, size},
                                      Color{0.85f, 0.85f, 1.0f, starAlpha(rng)});
        star->position = {coord(rng), coord(rng)};
        star->zIndex = -10;
    }

    std::uniform_real_distribution<float> asteroidSize(2.0f, 7.0f);
    std::uniform_real_distribution<float> spin(-1.0f, 1.0f);
    for (int placed = 0; placed < kExtraAsteroidCount;) {
        const Vec2 at{coord(rng), coord(rng)};
        if ((at - kSpawnPoint).length() < kSpawnClearRadius) {
            continue;
        }
        const float size = asteroidSize(rng);
        addChild<Asteroid>(Vec2{size, size}, spin(rng))->position = at;
        ++placed;
    }
}

void Level::onShipCrashed(PlayerShip& ship) { ship.respawn(kSpawnPoint); }
