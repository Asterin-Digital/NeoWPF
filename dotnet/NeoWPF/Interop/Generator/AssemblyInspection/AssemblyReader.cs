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
        bool? isStatic = false,
        IReadOnlyList<ParameterPassingMode>? parameterPassingModes = null,
        int? genericArity = null)
    {
        if (parameterPassingModes is not null && parameterPassingModes.Count != parameterTypeNames.Count)
            throw Error("NWB2014", $"Method '{name}' has a different number of parameter passing modes than parameter types.");

        var candidates = type.Methods
            .Where(method => !method.IsConstructor && method.Name == name &&
                (isStatic is null || method.IsStatic == isStatic.Value))
            .ToArray();
        var matches = candidates
            .Where(method =>
                (genericArity is null || method.GenericArity == genericArity) &&
                method.Parameters.Count == parameterTypeNames.Count &&
                method.Parameters.Select((parameter, index) =>
                    parameter.TypeName == parameterTypeNames[index] &&
                    MatchesPassingMode(parameter, parameterPassingModes?[index] ?? ParameterPassingMode.Value))
                    .All(matchesParameter => matchesParameter))
            .ToArray();

        if (matches.Length == 1) return matches[0];
        if (matches.Length > 1)
            throw AmbiguousMethodError(name, matches);
        if (candidates.Length == 0)
            throw Error("NWB2012", $"Method '{name}' was not found on '{type.FullName}'.");

        throw Error("NWB2014", $"Method '{name}' exists, but its requested signature does not match.");
    }

    public InspectedMethod ResolveMethodByName(InspectedType type, string name, bool? isStatic = null)
    {
        var candidates = type.Methods
            .Where(method => !method.IsConstructor && method.Name == name &&
                (isStatic is null || method.IsStatic == isStatic.Value))
            .ToArray();

        if (candidates.Length == 1)
            return candidates[0];
        if (candidates.Length == 0)
            throw Error("NWB2012", $"Method '{name}' was not found on '{type.FullName}'.");

        throw AmbiguousMethodError(name, candidates);
    }

    private static bool MatchesPassingMode(InspectedParameter parameter, ParameterPassingMode passingMode) => passingMode switch
    {
        ParameterPassingMode.Value => !parameter.IsByReference,
        ParameterPassingMode.Ref => parameter.IsByReference && !parameter.IsIn && !parameter.IsOut,
        ParameterPassingMode.In => parameter.IsByReference && parameter.IsIn && !parameter.IsOut,
        ParameterPassingMode.Out => parameter.IsByReference && parameter.IsOut,
        _ => false,
    };

    private static BindingParseException AmbiguousMethodError(string name, IReadOnlyList<InspectedMethod> candidates)
    {
        var signatures = candidates.Select(FormatSignature);
        return Error("NWB2013", $"Method '{name}' is ambiguous. Candidates: {string.Join("; ", signatures)}.");
    }

    private static string FormatSignature(InspectedMethod method)
    {
        var parameters = method.Parameters.Select(parameter =>
        {
            var passingMode = parameter.IsOut ? "out " : parameter.IsIn ? "in " : parameter.IsByReference ? "ref " : string.Empty;
            return $"{passingMode}{parameter.TypeName}";
        });
        var genericArity = method.GenericArity == 0 ? string.Empty : $"``{method.GenericArity}";
        var staticState = method.IsStatic ? "static " : string.Empty;
        return $"{staticState}{method.Name}{genericArity}({string.Join(", ", parameters)})";
    }

    private static BindingParseException Error(string id, string message) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, new Parsing.SourceLocation(string.Empty, 1, 1)));
}
