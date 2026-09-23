using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;

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
                throw Error("NWB3010", $"Type '{type.Name}' is unsupported.", type.Location),
            _ => throw Error("NWB3011", $"Type '{type.Name}' cannot be resolved.", type.Location),
        };
    }

    private CanonicalType ResolveArray(CanonicalType type, InspectedAssembly assembly)
    {
        if (type.Element is null || type.ArrayRank < 1)
            throw Error("NWB3013", "Array types require an element type and a positive rank.", type.Location);

        return CanonicalType.CreateArray(Resolve(type.Element, assembly), type.ArrayRank, type.Location);
    }

    private CanonicalType ResolveWrapped(CanonicalType type, CanonicalTypeKind kind, InspectedAssembly assembly)
    {
        if (type.Element is null)
            throw Error("NWB3014", "Wrapped types require an element type.", type.Location);

        return CanonicalType.Wrapped(kind, Resolve(type.Element, assembly), type.Location);
    }

    private CanonicalType ResolveTuple(CanonicalType type, InspectedAssembly assembly)
    {
        if (type.Arguments.Count < 2)
            throw Error("NWB3015", "Tuple types require at least two elements.", type.Location);

        return CanonicalType.Tuple(type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray(), type.Location);
    }

    private CanonicalType ResolveTask(CanonicalType type, InspectedAssembly assembly)
    {
        var arguments = type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray();
        if (arguments.Length > 1)
            throw Error("NWB3016", "Task and ValueTask types accept at most one result type.", type.Location);

        return CanonicalType.Named(type.Kind, type.Name, arguments, null, type.Location);
    }

    private CanonicalType ResolveNamed(CanonicalType type, InspectedAssembly assembly)
    {
        var inspected = assembly.Types.SingleOrDefault(candidate => candidate.FullName == type.Name);
        if (inspected is null)
            throw Error("NWB2011", $"Type '{type.Name}' was not found in '{assembly.Identity.Name}'.", type.Location);

        var kind = inspected.IsEnum
            ? CanonicalTypeKind.Enum
            : inspected.IsStruct
                ? CanonicalTypeKind.NamedStruct
                : inspected.IsDelegate
                    ? CanonicalTypeKind.Delegate
                    : inspected.GenericArity > 0 || type.Arguments.Count > 0
                        ? CanonicalTypeKind.Generic
                        : CanonicalTypeKind.NamedClass;
        var arguments = type.Arguments.Select(argument => Resolve(argument, assembly)).ToArray();
        if (inspected.GenericArity != arguments.Length && inspected.GenericArity > 0)
            throw Error("NWB3017", $"Type '{type.Name}' requires {inspected.GenericArity} generic arguments.", type.Location);

        return CanonicalType.Named(kind, inspected.FullName, arguments, assembly.Identity.Name, type.Location);
    }

    private static BindingParseException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
