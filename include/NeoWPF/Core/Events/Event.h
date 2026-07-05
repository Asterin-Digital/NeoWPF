#pragma once

#include <functional>
#include <vector>

namespace NeoWPF::Core {

template<typename... Args>
class Event {
public:
    using Delegate = std::function<void(Args...)>;

    Event& operator+=(const Delegate& handler) {
        m_handlers.push_back(handler);
        return *this;
    }

    Event& operator-=(const Delegate& handler) {
        return *this;
    }

    void Invoke(Args... args) const {
        for (const auto& handler : m_handlers) {
            if (handler) {
                handler(args...);
            }
        }
    }

private:
    std::vector<Delegate> m_handlers;
};

}
