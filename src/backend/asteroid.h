#pragma once

#include "engine/Node2D.h"

class Asteroid : public engine::Node2D {
public:
    Asteroid(engine::Vec2 size, float spinSpeed);

protected:
    void update(float dt) override;

private:
    float spinSpeed_;
};
