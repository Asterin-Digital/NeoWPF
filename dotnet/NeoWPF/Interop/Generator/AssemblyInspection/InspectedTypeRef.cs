using System.Reflection;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public enum InspectedTypeRefKind
{
    Named,
    GenericParameter,
    Array,
    ByReference,
    Pointer,
    FunctionPointer,
}

public sealed record InspectedTypeRef(
    InspectedTypeRefKind Kind,
    string Name,
    int GenericArity,
    IReadOnlyList<InspectedTypeRef> Arguments,
    InspectedTypeRef? Element,
    int ArrayRank,
    int GenericParameterPosition)
{
    public string DisplayName => Kind switch
    {
        InspectedTypeRefKind.Array when Element is not null =>
            $"{Element.DisplayName}[{new string(',', Math.Max(0, ArrayRank - 1))}]",
        InspectedTypeRefKind.ByReference when Element is not null => $"{Element.DisplayName}&",
        InspectedTypeRefKind.Pointer when Element is not null => $"{Element.DisplayName}*",
        InspectedTypeRefKind.Named when Arguments.Count != 0 =>
            $"{Name}<{string.Join(", ", Arguments.Select(argument => argument.DisplayName))}>",
        _ => Name,
    };

    public static InspectedTypeRef FromReflection(Type type)
    {
        if (type.IsByRef)
            return Wrapped(InspectedTypeRefKind.ByReference, FromReflection(type.GetElementType()!));
        if (type.IsPointer)
            return Wrapped(InspectedTypeRefKind.Pointer, FromReflection(type.GetElementType()!));
        if (type.IsFunctionPointer)
            return Create(InspectedTypeRefKind.FunctionPointer, type.ToString(), 0);
        if (type.IsArray)
            return new InspectedTypeRef(
                InspectedTypeRefKind.Array,
                string.Empty,
                0,
                Array.Empty<InspectedTypeRef>(),
                FromReflection(type.GetElementType()!),
                type.GetArrayRank(),
                -1);
        if (type.IsGenericParameter)
            return new InspectedTypeRef(
                InspectedTypeRefKind.GenericParameter,
                type.Name,
                0,
                Array.Empty<InspectedTypeRef>(),
                null,
                0,
                type.GenericParameterPosition);

        if (type.IsGenericType)
        {
            var definition = type.GetGenericTypeDefinition();
            return new InspectedTypeRef(
                InspectedTypeRefKind.Named,
                NormalizeName(definition.FullName ?? definition.Name),
                definition.GetGenericArguments().Length,
                type.GetGenericArguments().Select(FromReflection).ToArray(),
                null,
                0,
                -1);
        }

        return Create(InspectedTypeRefKind.Named, NormalizeName(type.FullName ?? type.Name), 0);
    }

    public static InspectedTypeRef Named(string fullName) =>
        Create(InspectedTypeRefKind.Named, NormalizeName(fullName), 0);

    private static InspectedTypeRef Wrapped(InspectedTypeRefKind kind, InspectedTypeRef element) =>
        new(kind, string.Empty, 0, Array.Empty<InspectedTypeRef>(), element, 0, -1);

    private static InspectedTypeRef Create(InspectedTypeRefKind kind, string name, int genericArity) =>
        new(kind, name, genericArity, Array.Empty<InspectedTypeRef>(), null, 0, -1);

    private static string NormalizeName(string name) => string.Join(".", name.Replace('+', '.').Split('.')
        .Select(segment =>
        {
            var arityMarker = segment.IndexOf('`');
            return arityMarker < 0 ? segment : segment[..arityMarker];
        }));
}