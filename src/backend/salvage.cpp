#include "backend/salvage.h"

#include "engine/Area2D.h"
#include "engine/Sprite.h"

using namespace engine;

Salvage::Salvage(Vec2 size, float spinSpeed) : spinSpeed_(spinSpeed) {
    name = "Salvage";
    addChild<Sprite>(ShapeKind::Rect, size, Color::blue());
    addChild<Area2D>(RectShape{size});
}

void Salvage::update(float dt) { rotation -= spinSpeed_ * dt; }
