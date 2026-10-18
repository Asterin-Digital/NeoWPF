using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Resolution;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class NativeBindingEmitter
{
    public static string Emit(
        BindingModel model,
        IReadOnlyList<ResolvedType> types,
        string fingerprint)
    {
        var typeDeclarations = string.Join("\n\n", types.Select(resolvedType =>
        {
            var simpleTypeName = resolvedType.Declaration.NativeName;
            var method = resolvedType.Methods.Single();
            var parameterName = NativeNameValidation.EscapeCppIdentifier(method.Method.Parameters.Single().Name);

            return $$"""
/*
    @summary
    Provides the generated native binding for {{resolvedType.Type.Name}}.
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

        @param {{parameterName}}
        Value passed to the managed method.

        @returns
        The managed result or an error.
    */
    NeoWPF::DotNet::ManagedResult<std::int32_t> {{method.Declaration.NativeName}}(
        std::int32_t {{parameterName}}) const;

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
""";
        }));

        return $$"""
/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostSession.h>

#include <cstdint>
#include <filesystem>

namespace {{model.NativeName ?? model.Name}}
{
{{typeDeclarations}}
}
""";
    }

}
