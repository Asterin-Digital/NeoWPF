/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <utility>

namespace NeoWPF {

/*
    @summary
    Adds optional, user-facing guidance to a derived type.

    @details
    Hints are short suggestions that help a user decide what to do next.
    Technical failure details belong in errors or diagnostics, not hints.

    @param T
    Type that inherits from Hints.
*/
template<typename T>
class Hints {
public:
    /*
        @summary
        Sets the hint associated with this object.

        @param hint
        Short, user-facing guidance.

        @returns
        This object.
    */
    T& SetHint(std::string hint)
    {
        m_hint = std::move(hint);
        return static_cast<T&>(*this);
    }

    /*
        @summary
        Gets the hint associated with this object.

        @returns
        The stored hint, or an empty string when no hint was set.
    */
    const std::string& GetHint() const noexcept
    {
        return m_hint;
    }

protected:
    Hints() = default;
    ~Hints() = default;

private:
    std::string m_hint;
};

}
