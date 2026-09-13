#pragma once

#include "engine/Node2D.h"

class Salvage : public engine::Node2D {
public:
    Salvage(engine::Vec2 size=engine::Vec2{1,1}, float spinSpeed=1);

protected:
    void update(float dt) override;

private:
    float spinSpeed_;
};
