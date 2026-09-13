#pragma once

#include <memory>

namespace engine {

enum class ConnectionType {
    Immediate,  // slot runs inside fire()
    Deferred,   // slot runs during SceneTree::tick(), after physics, before frees
};

namespace detail {
struct ConnectionState {
    bool connected = true;
};
}  // namespace detail

// Handle to a signal connection. Not RAII: a connection lasts until either
// side is freed or disconnect() is called.
class Connection {
public:
    Connection() = default;
    explicit Connection(std::weak_ptr<detail::ConnectionState> state)
        : state_(std::move(state)) {}

    void disconnect() {
        if (auto s = state_.lock()) {
            s->connected = false;
        }
    }
    bool connected() const {
        auto s = state_.lock();
        return s && s->connected;
    }

private:
    std::weak_ptr<detail::ConnectionState> state_;
};

}  // namespace engine
