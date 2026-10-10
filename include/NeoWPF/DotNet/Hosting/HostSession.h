#pragma once

/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#include <NeoWPF/DotNet/Abi/NwBridgeApi.h>
#include <NeoWPF/DotNet/Errors/ManagedResult.h>
#include <NeoWPF/DotNet/References/ManagedReference.h>

#include <cstdint>
#include <span>
#include <string_view>

namespace NeoWPF::DotNet::Hosting {

class Host;

/*
    @summary
    Represents the live state and bridge API for one .NET host session.
*/
class HostSession final {
public:
    /*
        @internal
    */
    bool IsAlive() const noexcept;

    /*
        @internal
    */
    std::uint32_t GetContextId() const noexcept;

    /*
        @internal
    */
    const NwBridgeApi* GetBridgeApi() const noexcept;

    /*
        @internal
    */
    ManagedResult<ManagedAssemblyReference> LoadAssembly(std::string_view path) const;

    /*
        @internal
    */
    ManagedResult<void> VerifyContract(
        std::string_view bindingName,
        std::span<const std::uint8_t, 32> fingerprint,
        std::span<const std::uint8_t> assemblyMvids) const;

    /*
        @internal
    */
    ManagedResult<ManagedTypeReference> ResolveType(
        std::string_view fullName,
        ManagedAssemblyReference assembly = {}) const;

    /*
        @internal
    */
    ManagedResult<ManagedMemberReference> ResolveMember(
        const ManagedTypeReference& type,
        std::string_view name,
        std::span<const ManagedTypeReference> parameterTypes,
        bool isStatic) const;

    /*
        @internal
    */
    ManagedResult<std::int32_t> InvokeInt32(
        const ManagedMemberReference& member,
        std::int32_t argument) const;

private:
    friend class Host;

    /*
        @internal
    */
    void Start(std::uint32_t contextId, const NwBridgeApi& bridgeApi) noexcept;

    /*
        @internal
    */
    void Stop(std::uint32_t drainTimeoutMilliseconds) noexcept;

    std::uint32_t m_contextId = 0;
    NwBridgeApi m_bridgeApi{};
};

}