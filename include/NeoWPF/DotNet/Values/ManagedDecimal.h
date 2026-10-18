/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <array>
#include <cstdint>

namespace NeoWPF::DotNet {

/*
    @summary
    Stores the four 32-bit words used by the managed decimal ABI value.
*/
class ManagedDecimal final {
public:
    constexpr ManagedDecimal() noexcept = default;

    constexpr ManagedDecimal(
        std::int32_t low,
        std::int32_t middle,
        std::int32_t high,
        std::int32_t flags) noexcept
        : m_low(low), m_middle(middle), m_high(high), m_flags(flags)
    {
    }

    /*
        @summary
        Gets the decimal words in managed GetBits order.
    */
    constexpr std::array<std::int32_t, 4> GetBits() const noexcept
    {
        return { m_low, m_middle, m_high, m_flags };
    }

private:
    std::int32_t m_low = 0;
    std::int32_t m_middle = 0;
    std::int32_t m_high = 0;
    std::int32_t m_flags = 0;
};

static_assert(sizeof(ManagedDecimal) == 16);
static_assert(alignof(ManagedDecimal) == 4);

}