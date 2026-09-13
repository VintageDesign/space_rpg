#include "engine/Input.h"

namespace engine {

void Input::keyEvent(int key, bool pressed) {
    if (pressed) {
        if (down_.insert(key).second) {
            justPressed_.insert(key);
        }
    } else {
        down_.erase(key);
    }
}

}  // namespace engine
