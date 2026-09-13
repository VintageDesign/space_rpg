#include "backend/asteroid.h"

#include "engine/Area2D.h"
#include "engine/Sprite.h"

using namespace engine;

Asteroid::Asteroid(Vec2 size, float spinSpeed) : spinSpeed_(spinSpeed) {
    name = "Asteroid";
    addChild<Sprite>(ShapeKind::Rect, size, Color::gray());
    addChild<Area2D>(RectShape{size});
}

void Asteroid::update(float dt) { rotation += spinSpeed_ * dt; }
