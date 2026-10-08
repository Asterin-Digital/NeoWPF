namespace NeoWPF.DotNet.Interop.Generator.Parsing;

internal static class BindingGrammar
{
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
        ["object"] = "reference"
    };

    private static IReadOnlySet<string> PrimitiveTypes { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "void", "bool", "int8", "uint8", "int16", "uint16", "int32", "uint32",
        "int64", "uint64", "float", "double", "decimal", "string", "bytes"
    };

    private static IReadOnlySet<string> PropertyModifiers { get; } = new HashSet<string>(StringComparer.Ordinal)
    {
        "readonly", "writeonly", "owned", "borrowed", "shared"
    };

    public static bool IsPrimitiveType(string name) => PrimitiveTypes.Contains(name);

    public static bool IsPropertyModifier(string value) => PropertyModifiers.Contains(value);
}
