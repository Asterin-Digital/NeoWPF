using System.Reflection;
using System.Runtime.InteropServices;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public sealed record AssemblyIdentity(
    string Name,
    Version? Version,
    string Culture,
    string PublicKeyToken,
    Guid ModuleVersionId,
    string Location);

public sealed record InspectedAssembly(
    AssemblyIdentity Identity,
    IReadOnlyList<InspectedType> Types,
    IReadOnlyList<AssemblyIdentity> Dependencies);

public sealed class AssemblyReader
{
    private static readonly Lazy<IReadOnlyList<InspectedType>> s_frameworkTypes =
        new(ReadFrameworkTypes, LazyThreadSafetyMode.ExecutionAndPublication);

    public InspectedAssembly Read(string assemblyPath, IReadOnlyList<string>? dependencyPaths = null)
    {
        var fullPath = Path.GetFullPath(assemblyPath);
        if (!File.Exists(fullPath))
            throw new FileNotFoundException("The assembly was not found.", fullPath);

        dependencyPaths ??= Array.Empty<string>();
        var explicitPaths = dependencyPaths
            .Select(Path.GetFullPath)
            .Prepend(fullPath)
            .ToArray();
        foreach (var dependencyPath in explicitPaths.Skip(1))
        {
            if (!File.Exists(dependencyPath))
                throw new FileNotFoundException("A declared dependency assembly was not found.", dependencyPath);
        }

        var searchDirectories = explicitPaths
            .Select(Path.GetDirectoryName)
            .Where(directory => directory is not null)
            .Cast<string>()
            .Distinct(StringComparer.OrdinalIgnoreCase);
        var adjacentPaths = searchDirectories.SelectMany(directory => Directory.GetFiles(directory, "*.dll"));
        var frameworkPaths = Directory.GetFiles(RuntimeEnvironment.GetRuntimeDirectory(), "*.dll");
        var resolverPaths = frameworkPaths
            .Concat(adjacentPaths)
            .Concat(explicitPaths)
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
            Convert.ToHexString(assembly.GetName().GetPublicKeyToken() ?? Array.Empty<byte>()).ToLowerInvariant(),
            assembly.ManifestModule.ModuleVersionId,
            fullPath);
        var loadedAssemblies = new List<Assembly>();
        foreach (var path in explicitPaths)
        {
            loadedAssemblies.Add(loadContext.LoadFromAssemblyPath(path));
        }

        var types = loadedAssemblies
            .SelectMany(candidateAssembly => candidateAssembly.GetExportedTypes()
                .Where(type => type.FullName is not null)
                .Select(type => MemberInspection.InspectType(type, delegateType)))
            .Concat(s_frameworkTypes.Value)
            .DistinctBy(type => (type.AssemblyName, type.FullName))
            .OrderBy(type => type.FullName, StringComparer.Ordinal)
            .ThenBy(type => type.AssemblyName, StringComparer.Ordinal)
            .ToArray();
        var dependencies = loadedAssemblies
            .Where(candidate => !string.Equals(candidate.Location, fullPath, StringComparison.OrdinalIgnoreCase))
            .Select(candidate => new AssemblyIdentity(
                candidate.GetName().Name ?? string.Empty,
                candidate.GetName().Version,
                candidate.GetName().CultureName ?? string.Empty,
                Convert.ToHexString(candidate.GetName().GetPublicKeyToken() ?? Array.Empty<byte>()).ToLowerInvariant(),
                candidate.ManifestModule.ModuleVersionId,
                candidate.Location))
            .DistinctBy(candidate => candidate.Location, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        return new InspectedAssembly(identity, types, dependencies);
    }

    private static IReadOnlyList<InspectedType> ReadFrameworkTypes()
    {
        var frameworkPaths = Directory.GetFiles(RuntimeEnvironment.GetRuntimeDirectory(), "*.dll");
        var resolver = new PathAssemblyResolver(frameworkPaths);
        using var loadContext = new MetadataLoadContext(resolver);
        var coreAssembly = loadContext.CoreAssembly
            ?? throw new InvalidOperationException("The metadata context does not expose its core assembly.");
        var delegateType = coreAssembly.GetType(typeof(MulticastDelegate).FullName!)
            ?? throw new InvalidOperationException("The metadata context does not expose MulticastDelegate.");
        var types = new List<InspectedType>();
        foreach (var path in frameworkPaths)
        {
            Assembly assembly;
            try
            {
                assembly = loadContext.LoadFromAssemblyPath(path);
            }
            catch (BadImageFormatException)
            {
                continue;
            }

            types.AddRange(assembly.GetExportedTypes()
                .Where(type => type.FullName is not null)
                .Select(type => MemberInspection.InspectType(type, delegateType)));
        }

        return types
            .OrderBy(type => type.FullName, StringComparer.Ordinal)
            .ThenBy(type => type.AssemblyName, StringComparer.Ordinal)
            .ToArray();
    }

    public InspectedType ResolveType(InspectedAssembly assembly, string fullName, SourceLocation location)
    {
        var normalizedName = NormalizeBindingTypeName(fullName);
        var candidates = assembly.Types
            .Where(candidate => string.Equals(NormalizeBindingTypeName(candidate.FullName), normalizedName, StringComparison.Ordinal))
            .ToArray();
        if (candidates.Length == 1)
            return candidates[0];
        if (candidates.Length > 1)
            throw Error(
                BindingDiagnosticIds.AmbiguousType,
                $"Type '{fullName}' is ambiguous across assemblies: {string.Join(", ", candidates.Select(candidate => candidate.AssemblyName).OrderBy(name => name, StringComparer.Ordinal))}.",
                location);
        throw Error(BindingDiagnosticIds.TypeNotFound, $"Type '{fullName}' was not found.", location);
    }

    public InspectedMethod ResolveMethod(
        InspectedType type,
        string name,
        IReadOnlyList<string> parameterTypeNames,
        SourceLocation location,
        bool? isStatic = null,
        IReadOnlyList<ParameterPassingMode>? parameterPassingModes = null,
        int? genericArity = null)
    {
        if (parameterPassingModes is not null && parameterPassingModes.Count != parameterTypeNames.Count)
            throw Error(BindingDiagnosticIds.ParameterPassingModeCount, $"Method '{name}' has a different number of parameter passing modes than parameter types.", location);

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
            throw AmbiguousMethodError(name, matches, location);
        if (candidates.Length == 0)
            throw Error(BindingDiagnosticIds.MethodNotFound, $"Method '{name}' was not found on '{type.FullName}'.", location);

        var signatures = string.Join("; ", candidates.Select(FormatSignature).OrderBy(value => value, StringComparer.Ordinal));
        throw Error(BindingDiagnosticIds.SignatureMismatch, $"Method '{name}' exists, but its requested signature does not match. Candidates: {signatures}.", location);
    }

    public InspectedMethod ResolveMethodBySignature(
        InspectedType type,
        string name,
        IReadOnlyList<CanonicalType> parameterTypes,
        SourceLocation location,
        bool? isStatic = null,
        IReadOnlyList<ParameterPassingMode>? parameterPassingModes = null,
        int? genericArity = null)
    {
        if (parameterPassingModes is not null && parameterPassingModes.Count != parameterTypes.Count)
        {
            throw Error(
                BindingDiagnosticIds.ParameterPassingModeCount,
                $"Method '{name}' has a different number of parameter passing modes than parameter types.",
                location);
        }

        var candidates = type.Methods
            .Where(method => !method.IsConstructor && method.Name == name &&
                (isStatic is null || method.IsStatic == isStatic.Value))
            .ToArray();
        var passingMatches = candidates
            .Where(method => method.Parameters.Count == parameterTypes.Count &&
                method.Parameters.Select((parameter, index) => MatchesPassingMode(
                    parameter,
                    parameterPassingModes?[index] ?? ParameterPassingMode.Value)).All(matches => matches))
            .ToArray();
        var matches = passingMatches
            .Where(method => method.Parameters.Select((parameter, index) =>
                TypeMatches(parameterTypes[index], parameter.Type)).All(matches => matches))
            .ToArray();

        if (matches.Length == 1)
            return matches[0];
        if (matches.Length > 1)
            throw AmbiguousMethodError(name, matches, location);
        if (candidates.Length == 0)
            throw Error(BindingDiagnosticIds.MethodNotFound, $"Method '{name}' was not found on '{type.FullName}'.", location);
        if (passingMatches.Length == 0)
        {
            var signatures = string.Join("; ", candidates.Select(FormatSignature).OrderBy(value => value, StringComparer.Ordinal));
            throw Error(BindingDiagnosticIds.SignatureMismatch, $"Method '{name}' has no matching parameter passing modes. Candidates: {signatures}.", location);
        }

        var mismatches = string.Join("; ", passingMatches.Select(FormatSignature).OrderBy(value => value, StringComparer.Ordinal));
        throw Error(BindingDiagnosticIds.ParameterTypeMismatch, $"Declared parameter types do not match '{name}'. Candidates: {mismatches}.", location);
    }

    public static bool TypeMatches(CanonicalType expected, InspectedTypeRef actual)
    {
        if (actual.Kind == InspectedTypeRefKind.ByReference)
            return actual.Element is not null && TypeMatches(expected, actual.Element);

        if (expected.Kind == CanonicalTypeKind.Array)
        {
            return actual.Kind == InspectedTypeRefKind.Array && actual.Element is not null &&
                actual.ArrayRank == expected.ArrayRank && expected.Element is not null &&
                TypeMatches(expected.Element, actual.Element);
        }

        if (expected.Kind == CanonicalTypeKind.Nullable)
        {
            return actual.Kind == InspectedTypeRefKind.Named &&
                actual.Name == "System.Nullable`1" && actual.Arguments.Count == 1 &&
                expected.Element is not null && TypeMatches(expected.Element, actual.Arguments[0]);
        }

        if (expected.Kind == CanonicalTypeKind.Tuple)
        {
            return actual.Kind == InspectedTypeRefKind.Named &&
                actual.Name == $"System.ValueTuple`{expected.Arguments.Count}" &&
                actual.Arguments.Count == expected.Arguments.Count &&
                expected.Arguments.Select((item, index) => TypeMatches(item, actual.Arguments[index])).All(matches => matches);
        }

        if (expected.Kind is CanonicalTypeKind.Generic or CanonicalTypeKind.Task or CanonicalTypeKind.ValueTask)
        {
            return actual.Kind == InspectedTypeRefKind.Named &&
                NormalizeBindingTypeName(actual.Name) == NormalizeBindingTypeName(expected.Name) &&
                actual.GenericArity == expected.Arguments.Count && actual.Arguments.Count == expected.Arguments.Count &&
                expected.Arguments.Select((item, index) => TypeMatches(item, actual.Arguments[index])).All(matches => matches);
        }

        if (actual.Kind == InspectedTypeRefKind.GenericParameter)
            return expected.Name == actual.Name;

        var expectedName = expected.Kind switch
        {
            CanonicalTypeKind.Void => "System.Void",
            CanonicalTypeKind.Boolean => "System.Boolean",
            CanonicalTypeKind.SignedInteger => expected.IntegerWidth switch
            {
                IntegerWidth.Int8 => "System.SByte",
                IntegerWidth.Int16 => "System.Int16",
                IntegerWidth.Int32 => "System.Int32",
                IntegerWidth.Int64 => "System.Int64",
                _ => string.Empty,
            },
            CanonicalTypeKind.UnsignedInteger => expected.IntegerWidth switch
            {
                IntegerWidth.UInt8 => "System.Byte",
                IntegerWidth.UInt16 => "System.UInt16",
                IntegerWidth.UInt32 => "System.UInt32",
                IntegerWidth.UInt64 => "System.UInt64",
                _ => string.Empty,
            },
            CanonicalTypeKind.FloatingPoint => expected.Name == "float" ? "System.Single" : "System.Double",
            CanonicalTypeKind.Decimal => "System.Decimal",
            CanonicalTypeKind.String => "System.String",
            CanonicalTypeKind.Bytes => "System.Byte[]",
            _ => NormalizeBindingTypeName(expected.Name),
        };

        return NormalizeBindingTypeName(actual.Name) == NormalizeBindingTypeName(expectedName);
    }

    public static bool AsyncResultMatches(CanonicalType expectedResult, InspectedTypeRef actualReturn)
    {
        if (actualReturn.Kind != InspectedTypeRefKind.Named ||
            actualReturn.GenericArity != 1 ||
            actualReturn.Arguments.Count != 1 ||
            actualReturn.Name is not ("System.Threading.Tasks.Task" or "System.Threading.Tasks.ValueTask"))
        {
            return false;
        }

        return TypeMatches(expectedResult, actualReturn.Arguments[0]);
    }

    public InspectedMethod ResolveMethodByName(InspectedType type, string name, SourceLocation location, bool? isStatic = null)
    {
        var candidates = type.Methods
            .Where(method => !method.IsConstructor && method.Name == name &&
                (isStatic is null || method.IsStatic == isStatic.Value))
            .ToArray();

        if (candidates.Length == 1)
            return candidates[0];
        if (candidates.Length == 0)
            throw Error(BindingDiagnosticIds.MethodNotFound, $"Method '{name}' was not found on '{type.FullName}'.", location);

        throw AmbiguousMethodError(name, candidates, location);
    }

    private static bool MatchesPassingMode(InspectedParameter parameter, ParameterPassingMode passingMode) => passingMode switch
    {
        ParameterPassingMode.Value => !parameter.IsByReference,
        ParameterPassingMode.Ref => parameter.IsByReference && !parameter.IsIn && !parameter.IsOut,
        ParameterPassingMode.In => parameter.IsByReference && parameter.IsIn && !parameter.IsOut,
        ParameterPassingMode.Out => parameter.IsByReference && parameter.IsOut,
        _ => false,
    };

    private static BindingDiagnosticException AmbiguousMethodError(
        string name,
        IReadOnlyList<InspectedMethod> candidates,
        SourceLocation location)
    {
        var signatures = candidates.Select(FormatSignature).OrderBy(value => value, StringComparer.Ordinal);
        return Error(BindingDiagnosticIds.AmbiguousMethod, $"Method '{name}' is ambiguous. Candidates: {string.Join("; ", signatures)}.", location);
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

    private static string NormalizeBindingTypeName(string name) => string.Join('.', name.Replace('+', '.').Split('.')
        .Select(segment =>
        {
            var arityMarker = segment.IndexOf('`');
            return arityMarker < 0 ? segment : segment[..arityMarker];
        }));

    private static BindingDiagnosticException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
