using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Resolution;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class NativeBindingSourceEmitter
{
    public static string Emit(
        BindingModel model,
        IReadOnlyList<ResolvedType> types,
        string fingerprint,
        IReadOnlyList<AssemblyIdentity> identities)
    {
        var fingerprintBytes = string.Join(", ", Convert.FromHexString(fingerprint).Select(value => $"0x{value:X2}"));
        var mvidBytes = string.Join(", ", identities.SelectMany(identity => identity.ModuleVersionId.ToByteArray())
            .Select(value => $"0x{value:X2}"));
        var typeImplementations = string.Join("\n\n", types.Select(resolvedType => EmitType(
            model,
            resolvedType,
            fingerprintBytes,
            mvidBytes,
            identities.Count * 16,
            identities)));

        return $$"""
#include <{{model.NativeName ?? model.Name}}.g.h>

#include <array>
#include <string>

namespace {
std::string ToUtf8Path(const std::filesystem::path& path)
{
    const auto value = path.u8string();
    return std::string(reinterpret_cast<const char*>(value.data()), value.size());
}
}

namespace {{model.NativeName ?? model.Name}}
{
{{typeImplementations}}
}
""";
    }

    private static string EmitType(
        BindingModel model,
        ResolvedType resolvedType,
        string fingerprintBytes,
        string mvidBytes,
        int assemblyMvidCount,
        IReadOnlyList<AssemblyIdentity> identities)
    {
        var simpleTypeName = resolvedType.Declaration.NativeName;
        var resolvedMethod = resolvedType.Methods.Single();
        var method = resolvedMethod.Method;
        var nativeMethodName = resolvedMethod.Declaration.NativeName;
        var parameterName = NativeNameValidation.EscapeCppIdentifier(method.Parameters.Single().Name);
        var dependencyLoads = string.Join("\n\n", identities.Skip(1).Select((identity, index) => $$"""
    const auto dependencyAssembly{{index}} = session.LoadAssembly(
        ToUtf8Path(bindingDirectory / "managed" / "{{Path.GetFileName(identity.Location)}}"));
    if (!dependencyAssembly{{index}}.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(dependencyAssembly{{index}}.GetMessage());
"""));

        return $$"""
NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding> {{simpleTypeName}}Binding::Load(
    const NeoWPF::DotNet::Hosting::HostSession& session,
    const std::filesystem::path& bindingDirectory)
{
    const auto managedAssembly = session.LoadAssembly(
        ToUtf8Path(bindingDirectory / "managed" / "{{Path.GetFileName(identities[0].Location)}}"));
    if (!managedAssembly.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(managedAssembly.GetMessage());

{{dependencyLoads}}
    const auto bindingAssembly = session.LoadAssembly(
        ToUtf8Path(bindingDirectory / "managed" / "{{BindingProjectEmitter.GetBindingAssemblyName(model.Name)}}.dll"));
    if (!bindingAssembly.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(bindingAssembly.GetMessage());

    constexpr std::array<std::uint8_t, 32> fingerprint{ {{fingerprintBytes}} };
    constexpr std::array<std::uint8_t, {{assemblyMvidCount}}> assemblyMvids{ {{mvidBytes}} };
    const auto contractStatus = session.VerifyContract(
        "{{model.Name}}",
        fingerprint,
        assemblyMvids);
    if (!contractStatus.Succeeded())
        return NeoWPF::DotNet::ManagedResult<{{simpleTypeName}}Binding>::Error(contractStatus.GetMessage());

    const auto managedType = session.ResolveType(
        "{{resolvedType.Type.Name}}",
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
""";
    }
}