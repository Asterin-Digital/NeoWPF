using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class ManagedBindingEmitter
{
    public static string Emit(
        BindingModel model,
        string fingerprint,
        IReadOnlyList<AssemblyIdentity> identities)
    {
        var assemblyNames = string.Join(", ", identities.Select(identity => $"\"{identity.Name}\""));
        var assemblyMvids = string.Join(", ", identities.Select(identity => $"\"{identity.ModuleVersionId:D}\""));
        return $$"""
namespace {{model.Name}};

public static class BindingContract
{
    public const string ContractFingerprint = "{{fingerprint}}";
    public static readonly string[] AssemblyNames = [{{assemblyNames}}];
    public static readonly string[] AssemblyMvids = [{{assemblyMvids}}];
}
""";
    }
}
