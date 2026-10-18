/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <optional>
#include <string>
#include <utility>

namespace NeoWPF::DotNet {

/*
    @summary
    Represents either a successful managed value or an error message.
*/
template<typename T>
class ManagedResult final {
public:
    /*
        @summary
        Creates a successful result containing the value.

        @param value
        Value returned by the managed operation.
    */
    static ManagedResult Success(T value)
    {
        ManagedResult result;
        result.m_value.emplace(std::move(value));
        return result;
    }

    /*
        @summary
        Creates a failed result with the supplied message.

        @param message
        Description of the failure.
    */
    static ManagedResult Error(std::string message)
    {
        ManagedResult result;
        result.m_message = std::move(message);
        return result;
    }

    /*
        @summary
        Indicates whether the result contains a value.
    */
    bool Succeeded() const noexcept
    {
        return m_value.has_value();
    }

    /*
        @summary
        Gets the optional result value.
    */
    const std::optional<T>& GetValue() const noexcept
    {
        return m_value;
    }

    /*
        @summary
        Gets the error message, or an empty string for a successful result.
    */
    const std::string& GetMessage() const noexcept
    {
        return m_message;
    }

private:
    ManagedResult() = default;

    std::optional<T> m_value;
    std::string m_message;
};

template<>
class ManagedResult<void> final {
public:
    /*
        @summary
        Creates a successful result.
    */
    static ManagedResult Success() noexcept
    {
        ManagedResult result;
        result.m_succeeded = true;
        return result;
    }

    /*
        @summary
        Creates a failed result with the supplied message.

        @param message
        Description of the failure.
    */
    static ManagedResult Error(std::string message)
    {
        ManagedResult result;
        result.m_message = std::move(message);
        return result;
    }

    /*
        @summary
        Indicates whether the operation succeeded.
    */
    bool Succeeded() const noexcept
    {
        return m_succeeded;
    }

    /*
        @summary
        Gets the error message, or an empty string for a successful result.
    */
    const std::string& GetMessage() const noexcept
    {
        return m_message;
    }

private:
    ManagedResult() = default;

    bool m_succeeded = false;
    std::string m_message;
};

}