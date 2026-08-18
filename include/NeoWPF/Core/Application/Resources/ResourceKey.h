#pragma once

#include <cstddef>
#include <cstdint>

namespace NeoWPF {

class ResourceKey {
public:
    constexpr ResourceKey(const char* name)
        : m_name(name)
        , m_hash(HashString(name))
    {
    }

    constexpr uint64_t Hash() const noexcept
    {
        return m_hash;
    }

    constexpr const char* Name() const noexcept
    {
        return m_name;
    }

private:
    static constexpr uint64_t HashString(const char* text) noexcept
    {
        uint64_t hash = 14695981039346656037ull;
        while (*text != '\0') {
            hash ^= static_cast<unsigned char>(*text++);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    const char* m_name;
    uint64_t m_hash;
};

}

#define NEOWPF_RESOURCE_KEY(str) NeoWPF::ResourceKey(str)
