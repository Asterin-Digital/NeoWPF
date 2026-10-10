using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public enum CanonicalTypeKind
{
    Void,
    Null,
    Boolean,
    SignedInteger,
    UnsignedInteger,
    FloatingPoint,
    Decimal,
    String,
    Bytes,
    Enum,
    Interface,
    NamedClass,
    NamedStruct,
    Array,
    Nullable,
    Generic,
    Task,
    ValueTask,
    Delegate,
    ByReference,
    Pointer,
    FunctionPointer,
    Tuple,
    Unknown,
    Unsupported,
}

public enum IntegerWidth
{
    None,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
}

public sealed record CanonicalType(
    CanonicalTypeKind Kind,
    string Name,
    IntegerWidth IntegerWidth,
    IReadOnlyList<CanonicalType> Arguments,
    CanonicalType? Element,
    int ArrayRank,
    string? AssemblyIdentity,
    SourceLocation Location)
{
    public static CanonicalType Primitive(
        CanonicalTypeKind kind,
        string name,
        IntegerWidth integerWidth,
        SourceLocation location) =>
        new(kind, name, integerWidth, System.Array.Empty<CanonicalType>(), null, 0, null, location);

    public static CanonicalType Named(
        CanonicalTypeKind kind,
        string name,
        IReadOnlyList<CanonicalType> arguments,
        string? assemblyIdentity,
        SourceLocation location) =>
        new(kind, name, IntegerWidth.None, arguments, null, 0, assemblyIdentity, location);

    public static CanonicalType Wrapped(
        CanonicalTypeKind kind,
        CanonicalType element,
        SourceLocation location) =>
        new(kind, string.Empty, IntegerWidth.None, System.Array.Empty<CanonicalType>(), element, 0, null, location);

    public static CanonicalType CreateArray(
        CanonicalType element,
        int rank,
        SourceLocation location) =>
        new(CanonicalTypeKind.Array, string.Empty, IntegerWidth.None, System.Array.Empty<CanonicalType>(), element, rank, null, location);

    public static CanonicalType Tuple(
        IReadOnlyList<CanonicalType> elements,
        SourceLocation location) =>
        new(CanonicalTypeKind.Tuple, string.Empty, IntegerWidth.None, elements, null, 0, null, location);
}
