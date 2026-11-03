/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace NeoWPF::Platform::Capabilities
{
    /*
        @summary
        Describes whether a declared capability is supported.

        @details
        Use `CapabilityStatus::Undeclared` when a capability name is absent from a group.
    */
    enum class CapabilitySupport : std::uint8_t
    {
        Unsupported,
        Supported
    };

    /*
        @summary
        Describes the result of looking up a capability.

        @details
        `Undeclared` means the capability name is absent, while `Unsupported` means it was declared but is unavailable.
    */
    enum class CapabilityStatus : std::uint8_t
    {
        Undeclared,
        Unsupported,
        Supported
    };

    /*
        @summary
        Pairs a capability name with its declared support.
    */
    struct Capability
    {
        /*
            @summary
            Name used to identify this capability in its group.
        */
        std::string_view Name;

        /*
            @summary
            Declared support state for this capability.
        */
        CapabilitySupport Support;
    };

    /*
        @summary
        Groups related capabilities and their minimum support requirement.
    */
    struct CapabilityGroup
    {
        /*
            @summary
            Name identifying this capability group.
        */
        std::string_view Name;

        /*
            @summary
            Capabilities evaluated as part of this group.
        */
        std::span<const Capability> Features;

        /*
            @summary
            Minimum number of supported features required for the group to pass.
        */
        std::size_t MinimumSupportedCount = 0;
    };

    /*
        @summary
        Returns the status declared by a capability.

        @param capability
        Capability to inspect.

        @returns
        Its declared supported or unsupported status.
    */
    [[nodiscard]] constexpr CapabilityStatus GetStatus(const Capability& capability) noexcept
    {
        return capability.Support == CapabilitySupport::Supported
            ? CapabilityStatus::Supported
            : CapabilityStatus::Unsupported;
    }

    /*
        @summary
        Reports whether a capability is supported.

        @param capability
        Capability to inspect.

        @returns
        `true` when the capability is supported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool IsSupported(const Capability& capability) noexcept
    {
        return GetStatus(capability) == CapabilityStatus::Supported;
    }

    /*
        @summary
        Reports whether a capability is unsupported.

        @param capability
        Capability to inspect.

        @returns
        `true` when the capability is unsupported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool IsUnsupported(const Capability& capability) noexcept
    {
        return GetStatus(capability) == CapabilityStatus::Unsupported;
    }

    /*
        @summary
        Finds a capability by name within a group.

        @param group
        Group to search.

        @param name
        Capability name to find.

        @returns
        A pointer to the matching capability, or `nullptr` when the name is not declared.
    */
    [[nodiscard]] constexpr const Capability* Find(
        const CapabilityGroup& group,
        std::string_view name) noexcept
    {
        for (const Capability& feature : group.Features)
        {
            if (feature.Name == name)
                return &feature;
        }

        return nullptr;
    }

    /*
        @summary
        Returns the status of a named capability in a group.

        @param group
        Group to search.

        @param name
        Capability name to find.

        @returns
        The capability's status, or `Undeclared` when the name is absent.
    */
    [[nodiscard]] constexpr CapabilityStatus GetStatus(
        const CapabilityGroup& group,
        std::string_view name) noexcept
    {
        const Capability* capability = Find(group, name);
        return capability != nullptr
            ? GetStatus(*capability)
            : CapabilityStatus::Undeclared;
    }

    /*
        @summary
        Reports whether a capability name is present in a group.

        @param group
        Group to search.

        @param name
        Capability name to find.

        @returns
        `true` when the name is declared in the group; otherwise `false`.
    */
    [[nodiscard]] constexpr bool IsDeclared(
        const CapabilityGroup& group,
        std::string_view name) noexcept
    {
        return Find(group, name) != nullptr;
    }

    /*
        @summary
        Reports whether a named capability in a group is supported.

        @param group
        Group to search.

        @param name
        Capability name to find.

        @returns
        `true` when the name is declared and supported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool IsSupported(
        const CapabilityGroup& group,
        std::string_view name) noexcept
    {
        return GetStatus(group, name) == CapabilityStatus::Supported;
    }

    /*
        @summary
        Reports whether a named capability in a group is unsupported.

        @param group
        Group to search.

        @param name
        Capability name to find.

        @returns
        `true` when the name is declared and unsupported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool IsUnsupported(
        const CapabilityGroup& group,
        std::string_view name) noexcept
    {
        return GetStatus(group, name) == CapabilityStatus::Unsupported;
    }

    /*
        @summary
        Counts the supported capabilities in a group.

        @param group
        Group to count.

        @returns
        The number of supported capabilities.
    */
    [[nodiscard]] constexpr std::size_t CountSupported(const CapabilityGroup& group) noexcept
    {
        std::size_t count = 0;

        for (const Capability& feature : group.Features)
        {
            if (IsSupported(feature))
                ++count;
        }

        return count;
    }

    /*
        @summary
        Counts the unsupported capabilities in a group.

        @param group
        Group to count.

        @returns
        The number of unsupported capabilities.
    */
    [[nodiscard]] constexpr std::size_t CountUnsupported(const CapabilityGroup& group) noexcept
    {
        return group.Features.size() - CountSupported(group);
    }

    /*
        @summary
        Reports whether a group contains any supported capability.

        @param group
        Group to inspect.

        @returns
        `true` when at least one capability is supported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool HasAnySupported(const CapabilityGroup& group) noexcept
    {
        return CountSupported(group) != 0;
    }

    /*
        @summary
        Reports whether every capability in a group is supported.

        @param group
        Group to inspect.

        @returns
        `true` when all capabilities are supported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool AreAllSupported(const CapabilityGroup& group) noexcept
    {
        return CountSupported(group) == group.Features.size();
    }

    /*
        @summary
        Reports whether a group has at least the requested number of supported capabilities.

        @param group
        Group to inspect.

        @param count
        Required number of supported capabilities.

        @returns
        `true` when the requested count is available and supported; otherwise `false`.
    */
    [[nodiscard]] constexpr bool HasAtLeastSupported(
        const CapabilityGroup& group,
        std::size_t count) noexcept
    {
        return count <= group.Features.size() && CountSupported(group) >= count;
    }

    /*
        @summary
        Reports whether a group meets its configured minimum support requirement.

        @param group
        Group to inspect.

        @returns
        `true` when the group has at least `MinimumSupportedCount` supported capabilities; otherwise `false`.
    */
    [[nodiscard]] constexpr bool MeetsRequirement(const CapabilityGroup& group) noexcept
    {
        return HasAtLeastSupported(group, group.MinimumSupportedCount);
    }
}
