using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public sealed class CanonicalTypeResolver
{
    public CanonicalType Resolve(CanonicalType type, InspectedAssembly assembly)
    {
        return type.Kind switch
        {
            CanonicalTypeKind.Void or
            CanonicalTypeKind.Boolean or
            CanonicalTypeKind.SignedInteger or
            CanonicalTypeKind.UnsignedInteger or
            CanonicalTypeKind.FloatingPoint or
            CanonicalTypeKind.Decimal or
            CanonicalTypeKind.String or
            CanonicalTypeKind.Bytes => type,
            CanonicalTypeKind.Array => ResolveArray(type, assembly),
            CanonicalTypeKind.Nullable => ResolveWrapped(type, CanonicalTypeKind.Nullable, assembly),
            CanonicalTypeKind.Tuple => ResolveTuple(type, assembly),
            CanonicalTypeKind.Task or CanonicalTypeKind.ValueTask => ResolveTask(type, assembly),
            CanonicalTypeKind.NamedClass or
            CanonicalTypeKind.NamedStruct or
            CanonicalTypeKind.Enum or
            CanonicalTypeKind.Delegate or
            CanonicalTypeKind.Generic => ResolveNamed(type, assembly),
            CanonicalTypeKind.Pointer or CanonicalTypeKind.FunctionPointer or CanonicalTypeKind.Unsupported =>
                throw Error(BindingDiagnosticIds.UnsupportedType, $"Type '{type.Name}' is unsupported.", type.Location),
            _ => throw Error(BindingDiagnosticIds.UnresolvedTypeShape, $"Type '{type.Name}' cannot be resolved.", type.Location),
        };
    }

    private CanonicalType ResolveArray(CanonicalType type, InspectedAssembly assembly)
    {
        if (type.Element is null || type.ArrayRank < 1)
            throw Error(BindingDiagnosticIds.InvalidArrayShape, "Array types require an element type and a positive rank.", type.Location);

        return CanonicalType.CreateArray(Resolve(type.Element, assembly), type.ArrayRank, type.Location);
    }

    private CanonicalType ResolveWrapped(CanonicalType type, CanonicalTypeKind kind, InspectedAssembly assembly)
    {
        if (type.Element is null)
            throw Error(BindingDiagnosticIds.InvalidWrappedType, "Wrapped types require an element type.", type.Location);

        return CanonicalType.Wrapped(kind, Resolve(type.Element, assembly), type.Location);
    }

    private CanonicalType ResolveTuple(CanonicalType type, InspectedAssembly assembly)
    {
        if (type.Arguments.Count < 2)
            throw Error(BindingDiagnosticIds.InvalidTupleShape, "Tuple types require at least two elements.", type.Location);

        return CanonicalType.Tuple(type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray(), type.Location);
    }

    private CanonicalType ResolveTask(CanonicalType type, InspectedAssembly assembly)
    {
        var arguments = type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray();
        if (arguments.Length > 1)
            throw Error(BindingDiagnosticIds.InvalidTaskShape, "Task and ValueTask types accept at most one result type.", type.Location);

        return CanonicalType.Named(type.Kind, type.Name, arguments, null, type.Location);
    }

    private CanonicalType ResolveNamed(CanonicalType type, InspectedAssembly assembly)
    {
        var normalizedName = NormalizeName(type.Name);
        var arguments = type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray();
        var nameCandidates = assembly.Types
            .Where(candidate => NormalizeName(candidate.FullName) == normalizedName)
            .Where(candidate => type.AssemblyIdentity is null || candidate.AssemblyName == type.AssemblyIdentity)
            .ToArray();
        var candidates = nameCandidates.Where(candidate => candidate.GenericArity == arguments.Length).ToArray();
        if (candidates.Length == 0)
        {
            if (nameCandidates.Length != 0)
            {
                var arities = string.Join(", ", nameCandidates.Select(candidate => candidate.GenericArity).Distinct().Order());
                throw Error(
                    BindingDiagnosticIds.GenericArityMismatch,
                    $"Type '{type.Name}' was given {arguments.Length} generic arguments; available arities: {arities}.",
                    type.Location);
            }
            throw Error(BindingDiagnosticIds.TypeNotFound, $"Type '{type.Name}' was not found in '{assembly.Identity.Name}'.", type.Location);
        }
        if (candidates.Length > 1)
            throw Error(
                BindingDiagnosticIds.AmbiguousType,
                $"Type '{type.Name}' is ambiguous across assemblies: {string.Join(", ", candidates.Select(candidate => candidate.AssemblyName).OrderBy(name => name, StringComparer.Ordinal))}.",
                type.Location);

        var inspected = candidates[0];
        if (inspected.IsByRefLike)
            throw Error(BindingDiagnosticIds.UnsupportedType, $"By-ref-like type '{type.Name}' is not supported.", type.Location);

        var kind = inspected.IsEnum
            ? CanonicalTypeKind.Enum
            : inspected.IsInterface
                ? CanonicalTypeKind.Interface
            : inspected.IsStruct
                ? CanonicalTypeKind.NamedStruct
                : inspected.IsDelegate
                    ? CanonicalTypeKind.Delegate
                    : inspected.GenericArity > 0 || type.Arguments.Count > 0
                        ? CanonicalTypeKind.Generic
                        : CanonicalTypeKind.NamedClass;
        return CanonicalType.Named(kind, inspected.FullName, arguments, inspected.AssemblyName, type.Location);
    }

    private static string NormalizeName(string name) => string.Join('.', name.Replace('+', '.').Split('.')
        .Select(segment =>
        {
            var arityMarker = segment.IndexOf('`');
            return arityMarker < 0 ? segment : segment[..arityMarker];
        }));

    private static BindingDiagnosticException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
