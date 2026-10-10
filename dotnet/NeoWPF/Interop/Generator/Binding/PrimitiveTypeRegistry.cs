namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed record PrimitiveTypeDefinition(
    string Name,
    CanonicalTypeKind Kind,
    IntegerWidth Width,
    string ClrName,
    string NativeType,
    uint AbiValueKind);

public static class PrimitiveTypeRegistry
{
    private static readonly IReadOnlyDictionary<string, PrimitiveTypeDefinition> s_types =
        new Dictionary<string, PrimitiveTypeDefinition>(StringComparer.Ordinal)
        {
            ["void"] = new("void", CanonicalTypeKind.Void, IntegerWidth.None, "System.Void", "void", 0),
            ["bool"] = new("bool", CanonicalTypeKind.Boolean, IntegerWidth.None, "System.Boolean", "bool", 1),
            ["int8"] = new("int8", CanonicalTypeKind.SignedInteger, IntegerWidth.Int8, "System.SByte", "std::int8_t", 2),
            ["int16"] = new("int16", CanonicalTypeKind.SignedInteger, IntegerWidth.Int16, "System.Int16", "std::int16_t", 3),
            ["int32"] = new("int32", CanonicalTypeKind.SignedInteger, IntegerWidth.Int32, "System.Int32", "std::int32_t", 4),
            ["int64"] = new("int64", CanonicalTypeKind.SignedInteger, IntegerWidth.Int64, "System.Int64", "std::int64_t", 5),
            ["uint8"] = new("uint8", CanonicalTypeKind.UnsignedInteger, IntegerWidth.UInt8, "System.Byte", "std::uint8_t", 6),
            ["uint16"] = new("uint16", CanonicalTypeKind.UnsignedInteger, IntegerWidth.UInt16, "System.UInt16", "std::uint16_t", 7),
            ["uint32"] = new("uint32", CanonicalTypeKind.UnsignedInteger, IntegerWidth.UInt32, "System.UInt32", "std::uint32_t", 8),
            ["uint64"] = new("uint64", CanonicalTypeKind.UnsignedInteger, IntegerWidth.UInt64, "System.UInt64", "std::uint64_t", 9),
            ["float"] = new("float", CanonicalTypeKind.FloatingPoint, IntegerWidth.None, "System.Single", "float", 10),
            ["double"] = new("double", CanonicalTypeKind.FloatingPoint, IntegerWidth.None, "System.Double", "double", 11),
            ["decimal"] = new("decimal", CanonicalTypeKind.Decimal, IntegerWidth.None, "System.Decimal", "NeoWPF::DotNet::ManagedDecimal", 12),
            ["string"] = new("string", CanonicalTypeKind.String, IntegerWidth.None, "System.String", "std::string", 13),
            ["bytes"] = new("bytes", CanonicalTypeKind.Bytes, IntegerWidth.None, "System.Byte[]", "std::vector<std::uint8_t>", 14),
        };

    public static IReadOnlyCollection<PrimitiveTypeDefinition> All { get; } = s_types.Values.ToArray();

    public static bool TryGet(string name, out PrimitiveTypeDefinition definition) =>
        s_types.TryGetValue(name, out definition!);

    public static PrimitiveTypeDefinition Get(string name) =>
        s_types.TryGetValue(name, out var definition)
            ? definition
            : throw new KeyNotFoundException($"'{name}' is not a registered primitive type.");
}
