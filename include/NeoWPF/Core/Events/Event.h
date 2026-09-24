/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace NeoWPF::Core {

template<typename... Args>
class Event {
public:
    using Delegate = std::function<void(Args...)>;

    Event& operator+=(const Delegate& handler) {
        m_handlers.push_back({handler, &handler});
        return *this;
    }

    Event& operator-=(const Delegate& handler) {
        const auto it = std::find_if(m_handlers.rbegin(), m_handlers.rend(),
            [&handler](const HandlerEntry& entry) {
                return entry.source == &handler;
            });
        if (it != m_handlers.rend()) {
            m_handlers.erase(std::next(it).base());
        }
        return *this;
    }

    void Invoke(Args... args) const {
        for (const auto& entry : m_handlers) {
            if (entry.handler) {
                entry.handler(args...);
            }
        }
    }

private:
    struct HandlerEntry {
        Delegate handler;
        const Delegate* source;
    };

    std::vector<HandlerEntry> m_handlers;
};

}
