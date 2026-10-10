using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

internal sealed class NativeNameValidation
{
    private static readonly HashSet<string> s_cppKeywords = new(StringComparer.Ordinal)
    {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool",
        "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
        "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast",
        "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete",
        "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
        "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
        "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
        "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
        "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
    };

    public IReadOnlyList<BindingDiagnostic> Validate(BindingModel model)
    {
        var diagnostics = new List<BindingDiagnostic>();
        ValidateName(model.NativeName ?? model.Name, model.Location, diagnostics);

        foreach (var type in model.Types)
        {
            ValidateName(type.NativeName, type.Location, diagnostics);
            foreach (var method in type.Methods)
                ValidateName(method.NativeName, method.Location, diagnostics);
        }

        foreach (var duplicate in model.Types.GroupBy(type => type.NativeName, StringComparer.Ordinal)
            .Where(group => group.Count() > 1))
        {
            foreach (var type in duplicate.Skip(1))
            {
                diagnostics.Add(new BindingDiagnostic(
                    BindingDiagnosticIds.EmittedNameCollision,
                    BindingDiagnosticSeverity.Error,
                    $"Multiple managed types emit the native name '{duplicate.Key}'.",
                    type.Location));
            }
        }

        return diagnostics;
    }

    public static string EscapeCppIdentifier(string identifier) =>
        s_cppKeywords.Contains(identifier) ? $"{identifier}_" : identifier;

    private static void ValidateName(string name, SourceLocation location, ICollection<BindingDiagnostic> diagnostics)
    {
        if (!IsIdentifier(name))
        {
            diagnostics.Add(new BindingDiagnostic(
                BindingDiagnosticIds.InvalidEmittedIdentifier,
                BindingDiagnosticSeverity.Error,
                $"'{name}' is not a valid emitted C++ identifier.",
                location));
        }
        else if (s_cppKeywords.Contains(name))
        {
            diagnostics.Add(new BindingDiagnostic(
                BindingDiagnosticIds.CppKeyword,
                BindingDiagnosticSeverity.Error,
                $"'{name}' is a C++ keyword and cannot be emitted as a name.",
                location));
        }
    }

    private static bool IsIdentifier(string name)
    {
        if (string.IsNullOrEmpty(name) || !(name[0] == '_' || IsAsciiLetter(name[0])))
            return false;

        return name.Skip(1).All(character => character == '_' || IsAsciiLetter(character) || char.IsAsciiDigit(character));
    }

    private static bool IsAsciiLetter(char character) =>
        character is >= 'A' and <= 'Z' or >= 'a' and <= 'z';
}