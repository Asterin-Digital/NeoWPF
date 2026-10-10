namespace NeoWPF.DotNet.Interop.Generator.Parsing;

internal static class BindingGrammar
{
    private static IReadOnlySet<string> PrimitiveTypeNames { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "void", "bool", "int8", "uint8", "int16", "uint16", "int32", "uint32",
        "int64", "uint64", "float", "double", "decimal", "string", "bytes"
    };

    public static IReadOnlySet<string> ReservedWords { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "binding", "assembly", "dependency", "adapter", "version", "nativeName", "managedName", "type", "constructible",
        "constructor", "method", "property", "event", "parameter", "returns", "async", "execution", "ownership", "callback",
        "readable", "writable", "readonly", "writeonly", "ref", "in", "out", "owned", "borrowed", "shared", "true", "false",
        "void", "bool", "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64", "float", "double", "decimal", "string", "bytes"
    };

    public static IReadOnlySet<string> OwnershipKinds { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "owned", "borrowed", "shared"
    };

    public static IReadOnlySet<string> ExecutionPolicies { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "Managed", "Calling", "Application", "Background"
    };

    public static IReadOnlySet<string> CallbackPolicies { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "Managed", "Application", "Background"
    };

    public static IReadOnlyDictionary<string, string> CSharpAliases { get; } = new Dictionary<string, string>(StringComparer.Ordinal)
    {
        ["int"] = "int32",
        ["long"] = "int64",
        ["byte"] = "uint8",
        ["short"] = "int16",
        ["ushort"] = "uint16",
        ["uint"] = "uint32",
        ["ulong"] = "uint64",
        ["sbyte"] = "int8",
    };

    public static IReadOnlySet<string> UnsupportedCSharpAliases { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "char", "object"
    };

    private static IReadOnlySet<string> PropertyModifiers { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "readonly", "writeonly", "owned", "borrowed", "shared"
    };

    public static bool IsPropertyModifier(string value) => PropertyModifiers.Contains(value);

    public static bool IsPrimitiveTypeName(string value) => PrimitiveTypeNames.Contains(value);
}
