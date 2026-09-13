#pragma once

#include "engine/Connection.h"
#include "engine/Node.h"
#include "engine/SceneTree.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine {

namespace detail {

// Deferred calls keep references to nodes (which outlive the flush, since
// frees run after it) but copy everything else, so a reference to a temporary
// never dangles.
template <class T>
using StoredArg = std::conditional_t<
    std::is_lvalue_reference_v<T> &&
        std::is_base_of_v<Node, std::remove_cv_t<std::remove_reference_t<T>>>,
    T, std::decay_t<T>>;

}  // namespace detail

// Typed signal for child -> parent events. Emitting while slots connect or
// disconnect (including nested emits) is safe.
template <class... Args>
class Signal {
public:
    Signal() = default;
    ~Signal() { disconnectAll(); }

    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    // Untracked: the caller keeps whatever the lambda captures alive.
    Connection connect(std::function<void(Args...)> fn) {
        return add(nullptr, std::move(fn), ConnectionType::Immediate);
    }

    // Auto-disconnects when `receiver` is freed. A deferred slot whose
    // receiver is outside a tree at fire time runs immediately.
    Connection connect(Node* receiver, std::function<void(Args...)> fn,
                       ConnectionType type = ConnectionType::Immediate) {
        return add(receiver, std::move(fn), type);
    }

    void fire(Args... args) {
        ++emitDepth_;
        const size_t count = slots_.size();
        for (size_t i = 0; i < count; ++i) {
            // Copy: a slot may connect, which can reallocate slots_.
            std::shared_ptr<Slot> slot = slots_[i];
            if (!slot->connected) {
                continue;
            }
            SceneTree* tree = slot->receiver ? slot->receiver->tree() : nullptr;
            if (slot->type == ConnectionType::Deferred && tree != nullptr) {
                tree->callDeferred(
                    [slot, stored = std::tuple<detail::StoredArg<Args>...>(
                               args...)]() mutable {
                        if (slot->connected) {
                            std::apply(slot->fn, stored);
                        }
                    });
            } else {
                slot->fn(args...);
            }
        }
        if (--emitDepth_ == 0) {
            compact();
        }
    }

    void disconnectAll() {
        for (auto& slot : slots_) {
            slot->connected = false;
        }
        if (emitDepth_ == 0) {
            slots_.clear();
        }
    }

    size_t connectionCount() const {
        return static_cast<size_t>(
            std::count_if(slots_.begin(), slots_.end(),
                          [](const auto& s) { return s->connected; }));
    }

private:
    struct Slot : detail::ConnectionState {
        std::function<void(Args...)> fn;
        Node* receiver = nullptr;
        ConnectionType type = ConnectionType::Immediate;
    };

    Connection add(Node* receiver, std::function<void(Args...)> fn,
                   ConnectionType type) {
        if (emitDepth_ == 0) {
            compact();
        }
        auto slot = std::make_shared<Slot>();
        slot->fn = std::move(fn);
        slot->receiver = receiver;
        slot->type = type;
        if (receiver != nullptr) {
            receiver->trackConnection(slot);
        }
        slots_.push_back(slot);
        return Connection(slot);
    }

    void compact() {
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
                                    [](const auto& s) { return !s->connected; }),
                     slots_.end());
    }

    std::vector<std::shared_ptr<Slot>> slots_;
    int emitDepth_ = 0;
};

// connect(child->damaged, this, &Parent::onDamaged)
template <class R, class... Args>
Connection connect(Signal<Args...>& signal, R* receiver,
                   void (R::*method)(Args...),
                   ConnectionType type = ConnectionType::Immediate) {
    return signal.connect(
        receiver,
        [receiver, method](Args... args) {
            (receiver->*method)(std::forward<Args>(args)...);
        },
        type);
}

}  // namespace engine
