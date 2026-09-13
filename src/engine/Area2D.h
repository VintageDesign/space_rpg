#pragma once

#include "engine/Node2D.h"
#include "engine/Shapes.h"
#include "engine/Signal.h"

#include <cstdint>
#include <vector>

namespace engine {

// Overlap detector. An area detects another when (mask & other.layer) != 0;
// only the detecting side emits areaEntered/areaExited and lists the other in
// overlapping(). Signals are emitted during SceneTree::tick(), after update().
class Area2D : public Node2D {
public:
    explicit Area2D(Shape shape = CircleShape{1.0f}) : shape(shape) {}

    Shape shape;
    uint32_t layer = 1;
    uint32_t mask = 1;

    // Provides the `other` Area2D
    Signal<Area2D&> areaEntered;

    // Provides the `other` Area2D
    Signal<Area2D&> areaExited;

    bool detects(const Area2D& other) const {
        return (mask & other.layer) != 0;
    }

    void testEntered(Area2D& other) {
      if (!detects(other)) {
        return;
      }
      overlapping_.push_back(&other);
      areaEntered.fire(other);
    }

    void testExited(Area2D& other) {
      if (!detects(other)) {
        return;
      }
      overlapping_.erase(
          std::remove(overlapping_.begin(), overlapping_.end(), &other),
          overlapping_.end());
      areaExited.fire(other);
    }
    const std::vector<Area2D*>& overlapping() const { return overlapping_; }

private:
    friend class SceneTree;
    std::vector<Area2D*> overlapping_;
};

}  // namespace engine
