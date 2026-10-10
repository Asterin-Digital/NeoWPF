using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class NativeBindingEmitter
{
    public static string Emit(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string nativeMethodName,
        string fingerprint)
    {
        var simpleTypeName = model.Types.Single(candidate => candidate.ManagedName == type.Name).NativeName;

        return $$"""
/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostSession.h>

#include <cstdint>
#include <filesystem>

namespace {{model.Name}}
{
/*
    @summary
    Provides the generated native binding for {{type.Name}}.
*/
class {{simpleTypeName}}Binding final
{
public:
    /*
        @summary
        Gets the contract fingerprint shared with the generated managed binding.
    */
    static constexpr const char* ContractFingerprint = "{{fingerprint}}";

    /*
        @summary
        Loads and validates the generated binding for a managed host session.

        @param session
        Live host session used for bridge operations.

        @param bindingDirectory
        Directory containing the managed assembly and generated binding assembly.

        @returns
        The resolved generated binding or an error.
    */
    static NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding> Load(
        const NeoWPF::DotNet::Hosting::HostSession& session,
        const std::filesystem::path& bindingDirectory);

    /*
        @summary
        Invokes the generated managed method.

        @param {{method.Parameters.Single().Name}}
        Value passed to the managed method.

        @returns
        The managed result or an error.
    */
    NeoWPF::DotNet::ManagedResult<std::int32_t> {{nativeMethodName}}(
        std::int32_t {{method.Parameters.Single().Name}}) const;

private:
    {{simpleTypeName}}Binding(
        const NeoWPF::DotNet::Hosting::HostSession& session,
        NeoWPF::DotNet::ManagedMemberReference member) noexcept
        : m_session(&session), m_member(member)
    {
    }

    const NeoWPF::DotNet::Hosting::HostSession* m_session;
    NeoWPF::DotNet::ManagedMemberReference m_member;
};
}
""";
    }

    public static string EmitImplementation(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string nativeMethodName,
        string fingerprint,
        AssemblyIdentity identity)
    {
        var simpleTypeName = model.Types.Single(candidate => candidate.ManagedName == type.Name).NativeName;
        var fingerprintBytes = string.Join(", ", Convert.FromHexString(fingerprint).Select(value => $"0x{value:X2}"));
        var mvidBytes = string.Join(", ", identity.ModuleVersionId.ToByteArray().Select(value => $"0x{value:X2}"));
        var parameterName = method.Parameters.Single().Name;

        return $$"""
#include "{{simpleTypeName}}.g.h"

#include <array>
#include <string>

namespace {{model.Name}}
{
NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding> {{simpleTypeName}}Binding::Load(
    const NeoWPF::DotNet::Hosting::HostSession& session,
    const std::filesystem::path& bindingDirectory)
{
    const auto managedAssembly = session.LoadAssembly(
        (bindingDirectory / "managed" / "{{identity.Name}}.dll").string());
    if (!managedAssembly.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(managedAssembly.GetMessage());

    const auto bindingAssembly = session.LoadAssembly(
        (bindingDirectory / "managed" / "{{model.Name}}.Bindings.dll").string());
    if (!bindingAssembly.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(bindingAssembly.GetMessage());

    constexpr std::array<std::uint8_t, 32> fingerprint{ {{fingerprintBytes}} };
    constexpr std::array<std::uint8_t, 16> assemblyMvid{ {{mvidBytes}} };
    const auto contractStatus = session.VerifyContract(
        "{{model.Name}}",
        fingerprint,
        assemblyMvid);
    if (!contractStatus.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(contractStatus.GetMessage());

    const auto managedType = session.ResolveType(
        "{{type.Name}}",
        managedAssembly.GetValue().value());
    if (!managedType.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(managedType.GetMessage());

    const auto parameterType = session.ResolveType("System.Int32");
    if (!parameterType.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(parameterType.GetMessage());

    const std::array<NeoWPF::DotNet::ManagedTypeReference, 1> parameterTypes{
        parameterType.GetValue().value()
    };
    const auto member = session.ResolveMember(
        managedType.GetValue().value(),
        "{{method.Name}}",
        parameterTypes,
        true);
    if (!member.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(member.GetMessage());

    return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Success(
        {{simpleTypeName}}Binding(session, member.GetValue().value()));
}

NeoWPF::DotNet::ManagedResult<std::int32_t> {{simpleTypeName}}Binding::{{nativeMethodName}}(
    std::int32_t {{parameterName}}) const
{
    return m_session->InvokeInt32(m_member, {{parameterName}});
}
}
""";
    }
}
