/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::DotNet {

enum class ManagedReferenceKind : std::uint32_t {
    None,
    Assembly,
    Type,
    Member,
    Object,
    Delegate,
    Exception,
    Operation,
};

/*
    @summary
    Stores a typed managed reference using the four-field native ABI layout.
*/
template<ManagedReferenceKind Kind>
class ManagedReference final {
public:
    constexpr ManagedReference() noexcept = default;

    constexpr ManagedReference(
        std::uint32_t contextId,
        std::uint32_t index,
        std::uint32_t generation) noexcept
        : m_contextId(contextId),
          m_index(index),
          m_generation(generation),
          m_kind(contextId == 0 && index == 0 && generation == 0
              ? ManagedReferenceKind::None
              : Kind)
    {
    }

    /*
        @summary
        Indicates whether the reference is null.
    */
    constexpr bool IsNull() const noexcept
    {
        return m_contextId == 0 && m_index == 0 && m_generation == 0 &&
            m_kind == ManagedReferenceKind::None;
    }

    /*
        @summary
        Gets the owning session identity.
    */
    constexpr std::uint32_t GetContextId() const noexcept
    {
        return m_contextId;
    }

    /*
        @summary
        Gets the managed reference table index.
    */
    constexpr std::uint32_t GetIndex() const noexcept
    {
        return m_index;
    }

    /*
        @summary
        Gets the managed reference generation.
    */
    constexpr std::uint32_t GetGeneration() const noexcept
    {
        return m_generation;
    }

    /*
        @summary
        Gets the managed reference kind.
    */
    constexpr ManagedReferenceKind GetKind() const noexcept
    {
        return m_kind;
    }

private:
    std::uint32_t m_contextId = 0;
    std::uint32_t m_index = 0;
    std::uint32_t m_generation = 0;
    ManagedReferenceKind m_kind = ManagedReferenceKind::None;
};

using ManagedAssemblyReference = ManagedReference<ManagedReferenceKind::Assembly>;
using ManagedTypeReference = ManagedReference<ManagedReferenceKind::Type>;
using ManagedMemberReference = ManagedReference<ManagedReferenceKind::Member>;
using ManagedObjectReference = ManagedReference<ManagedReferenceKind::Object>;
using ManagedDelegateReference = ManagedReference<ManagedReferenceKind::Delegate>;
using ManagedExceptionReference = ManagedReference<ManagedReferenceKind::Exception>;
using ManagedOperationReference = ManagedReference<ManagedReferenceKind::Operation>;

static_assert(sizeof(ManagedObjectReference) == 16);
static_assert(alignof(ManagedObjectReference) == 4);

}