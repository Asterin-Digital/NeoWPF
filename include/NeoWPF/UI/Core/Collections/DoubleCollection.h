/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace NeoWPF::UI::Core {

class DoubleCollection {
public:
    using value_type = double;
    using container_type = std::vector<value_type>;
    using const_iterator = container_type::const_iterator;

    DoubleCollection() = default;
    DoubleCollection(std::initializer_list<value_type> values)
        : m_values(values) {}

    std::size_t GetCount() const noexcept { return m_values.size(); }
    bool IsEmpty() const noexcept { return m_values.empty(); }

    const value_type& operator[](std::size_t index) const noexcept { return m_values[index]; }

    const_iterator begin() const noexcept { return m_values.begin(); }
    const_iterator end() const noexcept { return m_values.end(); }

    void Add(value_type value) { m_values.push_back(value); }
    void Clear() noexcept { m_values.clear(); }

    bool operator==(const DoubleCollection& other) const noexcept {
        return m_values == other.m_values;
    }

    bool operator!=(const DoubleCollection& other) const noexcept {
        return !(*this == other);
    }

private:
    container_type m_values;
};

}
