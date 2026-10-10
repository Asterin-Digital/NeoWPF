using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Parsing;

public sealed partial class BindingParser
{
    private enum MemberKind
    {
        Constructor,
        Method,
        Property,
        Event,
    }

    private readonly IReadOnlyList<BindingToken> _tokens;
    private int _index;

    public BindingParser(string text, string file = "<input>")
    {
        _tokens = new BindingLexer(text, file).Lex();
    }

    public BindingDocument ParseDocument()
    {
        var bindings = new List<BindingDeclaration>();
        while (!Check(BindingTokenKind.End))
            bindings.Add(ParseBinding());
        return new BindingDocument(bindings);
    }

    private BindingDeclaration ParseBinding()
    {
        var location = Expect("binding").Location;
        var name = ExpectIdentifier("binding name");
        ExpectSymbol("{");
        var items = new List<BindingItem>();
        while (!CheckSymbol("}") && !Check(BindingTokenKind.End))
            items.Add(ParseBindingItem());
        ExpectSymbol("}");
        CheckDuplicate(items.OfType<AssemblyDeclaration>(), "assembly");
        CheckDuplicate(items.OfType<VersionDeclaration>(), "version");
        CheckDuplicate(items.OfType<NativeNameDeclaration>(), "nativeName");
        return new BindingDeclaration(name.Text, items, location);
    }

    private BindingItem ParseBindingItem()
    {
        if (IsKeyword("assembly")) return ParsePathItem((path, location) => new AssemblyDeclaration(path, location));
        if (IsKeyword("dependency")) return ParsePathItem((path, location) => new DependencyDeclaration(path, location));
        if (IsKeyword("adapter")) return ParsePathItem((path, location) => new AdapterDeclaration(path, location));
        if (IsKeyword("version")) return ParsePathItem((path, location) => new VersionDeclaration(path, location));
        if (IsKeyword("nativeName")) return ParseNameItem((value, location) => new NativeNameDeclaration(value, location));
        if (IsKeyword("type")) return ParseType();
        throw Error(BindingDiagnosticIds.UnexpectedToken, $"Unexpected binding item '{Current.Text}'.");
    }

    private TypeDeclaration ParseType()
    {
        var location = Expect("type").Location;
        var name = ParseQualifiedName();
        ExpectSymbol("{");
        var items = new List<TypeItem>();
        while (!CheckSymbol("}") && !Check(BindingTokenKind.End))
        {
            if (IsKeyword("nativeName")) items.Add(ParseTypeNativeName());
            else if (IsKeyword("constructible")) items.Add(ParseConstructible());
            else if (IsKeyword("constructor")) items.Add(ParseConstructor());
            else if (IsKeyword("method")) items.Add(ParseMethod());
            else if (IsKeyword("property")) items.Add(ParseProperty());
            else if (IsKeyword("event")) items.Add(ParseEvent());
            else throw Error(BindingDiagnosticIds.UnexpectedToken, $"Unexpected type item '{Current.Text}'.");
        }
        ExpectSymbol("}");
        CheckDuplicate(items.OfType<TypeNativeNameDeclaration>(), "nativeName");
        CheckDuplicate(items.OfType<ConstructibleDeclaration>(), "constructible");
        return new TypeDeclaration(name, items, location);
    }

    private TypeNativeNameDeclaration ParseTypeNativeName()
    {
        var location = Expect("nativeName").Location;
        var name = Expect(BindingTokenKind.String, "native name").Text;
        ExpectSymbol(";");
        return new TypeNativeNameDeclaration(name, location);
    }

    private ConstructibleDeclaration ParseConstructible()
    {
        var location = Expect("constructible").Location;
        ExpectSymbol(";");
        return new ConstructibleDeclaration(location);
    }

    private TypeSyntax ParseTypeExpression()
    {
        var location = Current.Location;
        TypeSyntax type;
        if (Match("("))
        {
            var elements = new List<TypeSyntax> { ParseTypeExpression() };
            while (Match(",")) elements.Add(ParseTypeExpression());
            if (elements.Count < 2)
                throw Error(BindingDiagnosticIds.InvalidTuple, "Tuple types require at least two elements.");
            ExpectSymbol(")");
            type = new TupleTypeSyntax(elements, location);
        }
        else
        {
            var token = Current;
            if (!token.IsEscapedIdentifier && BindingGrammar.CSharpAliases.TryGetValue(token.Text, out var replacement))
                throw Error(BindingDiagnosticIds.CSharpKeywordAlias, $"C# keyword alias '{token.Text}' is not a primitive type; use '{replacement}'.");
            if (!token.IsEscapedIdentifier && BindingGrammar.UnsupportedCSharpAliases.Contains(token.Text))
                throw Error(BindingDiagnosticIds.CSharpKeywordAlias, $"C# keyword alias '{token.Text}' is not supported as a type name.");
            var name = ParseQualifiedTypeName();
            type = new NamedTypeSyntax(name, Array.Empty<TypeSyntax>(), location);

            if (Match("<"))
            {
                if (BindingGrammar.IsPrimitiveTypeName(name))
                    throw Error(BindingDiagnosticIds.UnexpectedToken, "Primitive types cannot declare generic arguments.");
                var arguments = new List<TypeSyntax> { ParseTypeExpression() };
                while (Match(",")) arguments.Add(ParseTypeExpression());
                ExpectSymbol(">");
                type = new NamedTypeSyntax(name, arguments, location);
            }
        }

        while (true)
        {
            if (Match("?")) type = new NullableTypeSyntax(type, location);
            else if (Match("["))
            {
                var rank = 1;
                while (Match(",")) rank++;
                ExpectSymbol("]");
                type = new ArrayTypeSyntax(type, rank, location);
            }
            else if (Match("*")) type = new PointerTypeSyntax(type, location);
            else break;
        }
        return type;
    }

    private string ParseQualifiedName()
    {
        var name = ExpectIdentifier("qualified name").Text;
        while (Match(".")) name += "." + ExpectIdentifier("qualified name segment").Text;
        return name;
    }

    private string ParseQualifiedTypeName()
    {
        var first = Current;
        var name = !first.IsEscapedIdentifier && BindingGrammar.IsPrimitiveTypeName(first.Text)
            ? Advance().Text
            : ExpectIdentifier("qualified type name").Text;
        while (Match(".")) name += "." + ExpectIdentifier("qualified type name segment").Text;
        return name;
    }

    private string ExpectContextualValue(string description) => Expect(BindingTokenKind.Identifier, description).Text;

    private T ParsePathItem<T>(Func<string, SourceLocation, T> factory) where T : BindingItem
    {
        var location = Current.Location;
        Advance();
        var path = Expect(BindingTokenKind.String, "path").Text;
        ExpectSymbol(";");
        return factory(path, location);
    }

    private T ParseNameItem<T>(Func<string, SourceLocation, T> factory) where T : BindingItem
    {
        var location = Current.Location;
        Advance();
        var name = Expect(BindingTokenKind.String, "name").Text;
        ExpectSymbol(";");
        return factory(name, location);
    }

    private ManagedOwnership? MatchOwnership()
    {
        if (!IsOwnershipKeyword()) return null;
        var value = Current.Text switch
        {
            "owned" => ManagedOwnership.Owned,
            "borrowed" => ManagedOwnership.Borrowed,
            "shared" => ManagedOwnership.Shared,
            _ => throw Error(BindingDiagnosticIds.UnknownOwnership, $"Unknown ownership kind '{Current.Text}'."),
        };
        Advance();
        return value;
    }

    private bool IsOwnershipKeyword() => IsKeyword("owned") || IsKeyword("borrowed") || IsKeyword("shared");

    private bool IsPropertyModifier() =>
        IsKeyword("readonly") || IsKeyword("writeonly") || IsOwnershipKeyword();

    private bool IsKeyword(string text) =>
        Current.Kind == BindingTokenKind.Identifier && !Current.IsEscapedIdentifier && Current.Text == text;

    private BindingToken Current => _tokens[_index];
    private bool Check(BindingTokenKind kind) => Current.Kind == kind;
    private bool CheckSymbol(string symbol) => Current.Kind == BindingTokenKind.Symbol && Current.Text == symbol;

    private bool Match(string text)
    {
        var isSymbol = text.Length == 1 && "{}();,.<>?[]:*".Contains(text[0]);
        if (isSymbol ? !CheckSymbol(text) : !IsKeyword(text)) return false;
        Advance();
        return true;
    }

    private BindingToken Advance() => _tokens[_index++];

    private BindingToken Expect(string text)
    {
        if (!Match(text)) throw Error(BindingDiagnosticIds.UnexpectedToken, $"Expected '{text}'.");
        return _tokens[_index - 1];
    }

    private BindingToken Expect(BindingTokenKind kind, string description)
    {
        if (Current.Kind != kind) throw Error(BindingDiagnosticIds.UnexpectedToken, $"Expected {description}.");
        return Advance();
    }

    private BindingToken ExpectIdentifier(string description)
    {
        var token = Expect(BindingTokenKind.Identifier, description);
        if (!token.IsEscapedIdentifier && BindingGrammar.ReservedWords.Contains(token.Text))
        {
            throw new BindingDiagnosticException(new BindingDiagnostic(
                BindingDiagnosticIds.EscapedIdentifierRequired,
                BindingDiagnosticSeverity.Error,
                $"'{token.Text}' is reserved here; use an @-escaped identifier.",
                token.Location));
        }
        return token.IsEscapedIdentifier ? token with { Text = token.Text[1..] } : token;
    }

    private void ExpectSymbol(string symbol) => Expect(symbol);

    private BindingDiagnosticException Error(string id, string message) =>
        Error(id, message, Current.Location);

    private static BindingDiagnosticException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
