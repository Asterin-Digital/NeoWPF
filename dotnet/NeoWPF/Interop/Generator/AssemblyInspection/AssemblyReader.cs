using System.Reflection;
using System.Runtime.InteropServices;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public sealed record AssemblyIdentity(
    string Name,
    Version? Version,
    string Culture,
    Guid ModuleVersionId,
    string Location);

public sealed record InspectedAssembly(
    AssemblyIdentity Identity,
    IReadOnlyList<InspectedType> Types);

public sealed class AssemblyReader
{
    public InspectedAssembly Read(string assemblyPath)
    {
        var fullPath = Path.GetFullPath(assemblyPath);
        if (!File.Exists(fullPath))
            throw new FileNotFoundException("The assembly was not found.", fullPath);

        var resolverPaths = Directory.GetFiles(RuntimeEnvironment.GetRuntimeDirectory(), "*.dll")
            .Append(fullPath)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToArray();
        var resolver = new PathAssemblyResolver(resolverPaths);

        using var loadContext = new MetadataLoadContext(resolver);
        var assembly = loadContext.LoadFromAssemblyPath(fullPath);
        var coreAssembly = loadContext.CoreAssembly
            ?? throw new InvalidOperationException("The metadata context does not expose its core assembly.");
        var delegateType = coreAssembly.GetType(typeof(MulticastDelegate).FullName!);
        if (delegateType is null)
            throw new InvalidOperationException("The metadata context does not expose MulticastDelegate.");
        var identity = new AssemblyIdentity(
            assembly.GetName().Name ?? string.Empty,
            assembly.GetName().Version,
            assembly.GetName().CultureName ?? string.Empty,
            assembly.ManifestModule.ModuleVersionId,
            fullPath);
        var types = assembly.GetExportedTypes()
            .Where(type => type.FullName is not null)
            .OrderBy(type => type.FullName, StringComparer.Ordinal)
            .Select(type => MemberInspection.InspectType(type, delegateType))
            .ToArray();

        return new InspectedAssembly(identity, types);
    }

    public InspectedType ResolveType(InspectedAssembly assembly, string fullName)
    {
        var type = assembly.Types.SingleOrDefault(candidate => candidate.FullName == fullName);
        return type ?? throw Error("NWB2011", $"Type '{fullName}' was not found.");
    }

    public InspectedMethod ResolveMethod(
        InspectedType type,
        string name,
        IReadOnlyList<string> parameterTypeNames,
        bool isStatic = false)
    {
        var candidates = type.Methods
            .Where(method => !method.IsConstructor && method.Name == name && method.IsStatic == isStatic)
            .ToArray();
        var matches = candidates
            .Where(method => method.Parameters.Select(parameter => parameter.TypeName).SequenceEqual(parameterTypeNames, StringComparer.Ordinal))
            .ToArray();

        if (matches.Length == 1) return matches[0];
        if (matches.Length > 1)
            throw Error("NWB2013", $"Method '{name}' has more than one matching signature.");
        if (candidates.Length == 0)
            throw Error("NWB2012", $"Method '{name}' was not found on '{type.FullName}'.");

        throw Error("NWB2014", $"Method '{name}' exists, but its requested signature does not match.");
    }

    private static BindingParseException Error(string id, string message) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, new Parsing.SourceLocation(string.Empty, 1, 1)));
}
