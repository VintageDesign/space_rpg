#pragma once

#include <Qt>

#include <unordered_set>

namespace engine {

// Keyboard state sampled once per frame by SceneTree::tick().
class Input {
public:
    bool isKeyDown(Qt::Key key) const { return down_.count(key) != 0; }
    bool isKeyJustPressed(Qt::Key key) const {
        return justPressed_.count(key) != 0;
    }

    void keyEvent(int key, bool pressed);
    void endFrame() { justPressed_.clear(); }

private:
    std::unordered_set<int> down_;
    std::unordered_set<int> justPressed_;
};

}  // namespace engine
