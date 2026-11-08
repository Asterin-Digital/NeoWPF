/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <utility>
#include <variant>

namespace NeoWPF {

/*
    @summary
    Holds a successful result value.

    @param T
    Type of the value returned on success.
*/
template<typename T>
struct Success final {
    T value;
};

/*
    @summary
    Marks a successful result with no value.
*/
template<>
struct Success<void> final {};

/*
    @summary
    Holds a failed result error.

    @param E
    Type of the error returned on failure.
*/
template<typename E>
struct Fail final {
    E error;
};

/*
    @summary
    Represents either a successful value or a failure.

    @param T
    Type of the value returned on success.

    @param E
    Type of the error returned on failure. Defaults to std::string.
*/
template<typename T, typename E = std::string>
class Result final {
public:
    Result(NeoWPF::Success<T> success)
        : state_(std::move(success))
    {
    }

    Result(NeoWPF::Fail<E> failure)
        : state_(std::move(failure))
    {
    }

    /*
        @summary
        Indicates whether the operation succeeded.
    */
    bool Success() const noexcept
    {
        return std::holds_alternative<NeoWPF::Success<T>>(state_);
    }

    /*
        @summary
        Indicates whether the operation failed.
    */
    bool Fail() const noexcept
    {
        return std::holds_alternative<NeoWPF::Fail<E>>(state_);
    }

    /*
        @summary
        Returns the successful value.

        @details
        Throws std::bad_variant_access when the result contains a failure.
    */
    T& Value() &
    {
        return std::get<NeoWPF::Success<T>>(state_).value;
    }

    /*
        @summary
        Returns the successful value.

        @details
        Throws std::bad_variant_access when the result contains a failure.
    */
    const T& Value() const &
    {
        return std::get<NeoWPF::Success<T>>(state_).value;
    }

    /*
        @summary
        Returns the successful value.

        @details
        Throws std::bad_variant_access when the result contains a failure.
    */
    T&& Value() &&
    {
        return std::get<NeoWPF::Success<T>>(std::move(state_)).value;
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the result contains a successful value.
    */
    E& Error() &
    {
        return std::get<NeoWPF::Fail<E>>(state_).error;
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the result contains a successful value.
    */
    const E& Error() const &
    {
        return std::get<NeoWPF::Fail<E>>(state_).error;
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the result contains a successful value.
    */
    E&& Error() &&
    {
        return std::get<NeoWPF::Fail<E>>(std::move(state_)).error;
    }

private:
    std::variant<NeoWPF::Success<T>, NeoWPF::Fail<E>> state_;
};

template<typename E>
class Result<void, E> final {
public:
    Result(NeoWPF::Success<void> success)
        : state_(std::move(success))
    {
    }

    Result(NeoWPF::Fail<E> failure)
        : state_(std::move(failure))
    {
    }

    /*
        @summary
        Indicates whether the operation succeeded.
    */
    bool Success() const noexcept
    {
        return std::holds_alternative<NeoWPF::Success<void>>(state_);
    }

    /*
        @summary
        Indicates whether the operation failed.
    */
    bool Fail() const noexcept
    {
        return std::holds_alternative<NeoWPF::Fail<E>>(state_);
    }

    /*
        @summary
        Checks that the operation succeeded.

        @details
        Throws std::bad_variant_access when the result contains a failure.
    */
    void Value() const
    {
        static_cast<void>(std::get<NeoWPF::Success<void>>(state_));
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the operation succeeded.
    */
    E& Error() &
    {
        return std::get<NeoWPF::Fail<E>>(state_).error;
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the operation succeeded.
    */
    const E& Error() const &
    {
        return std::get<NeoWPF::Fail<E>>(state_).error;
    }

    /*
        @summary
        Returns the failure error.

        @details
        Throws std::bad_variant_access when the operation succeeded.
    */
    E&& Error() &&
    {
        return std::get<NeoWPF::Fail<E>>(std::move(state_)).error;
    }

private:
    std::variant<NeoWPF::Success<void>, NeoWPF::Fail<E>> state_;
};

}
