using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Parsing;

public sealed class BindingParser
{
    private readonly IReadOnlyList<BindingToken> _tokens;
    private int _index;

    public BindingParser(string text, string file = "<input>")
    {
        _tokens = new BindingLexer(text, file).Lex();
    }

    public BindingDocument ParseDocument()
    {
        var bindings = new List<BindingDeclaration>();
        while (!Check(BindingTokenKind.End)) bindings.Add(ParseBinding());
        return new BindingDocument(bindings);
    }

    private BindingDeclaration ParseBinding()
    {
        var location = Expect("binding").Location;
        var name = ExpectIdentifier("binding name");
        ExpectSymbol("{");
        var items = new List<BindingItem>();
        while (!CheckSymbol("}")) items.Add(ParseBindingItem());
        ExpectSymbol("}");
        return new BindingDeclaration(name.Text, items, location);
    }

    private BindingItem ParseBindingItem() => Current.Text switch
    {
        "assembly" => ParsePathItem((path, location) => new AssemblyDeclaration(path, location)),
        "dependency" => ParsePathItem((path, location) => new DependencyDeclaration(path, location)),
        "adapter" => ParsePathItem((path, location) => new AdapterDeclaration(path, location)),
        "version" => ParsePathItem((path, location) => new VersionDeclaration(path, location)),
        "nativeName" => ParseNameItem((name, location) => new NativeNameDeclaration(name, location)),
        "type" => ParseType(),
        _ => throw Error("NWB1010", $"Unexpected binding item '{Current.Text}'."),
    };

    private TypeDeclaration ParseType()
    {
        var location = Expect("type").Location;
        var name = ParseQualifiedName();
        ExpectSymbol("{");
        var items = new List<TypeItem>();
        while (!CheckSymbol("}"))
        {
            items.Add(Current.Text switch
            {
                "nativeName" => ParseTypeNativeName(),
                "constructible" => ParseConstructible(),
                "constructor" => ParseConstructor(),
                "method" => ParseMethod(),
                "property" => ParseProperty(),
                "event" => ParseEvent(),
                _ => throw Error("NWB1010", $"Unexpected type item '{Current.Text}'."),
            });
        }
        ExpectSymbol("}");
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

    private ConstructorDeclaration ParseConstructor()
    {
        var location = Expect("constructor").Location;
        var parameters = ParseParameters();
        var items = ParseMemberBlockOrSemicolon();
        return new ConstructorDeclaration(parameters, items, location);
    }

    private MethodDeclaration ParseMethod()
    {
        var location = Expect("method").Location;
        var name = ExpectIdentifier("method name").Text;
        var hasExplicitSignature = CheckSymbol("(");
        var parameters = hasExplicitSignature ? ParseParameters() : Array.Empty<ParameterSyntax>();
        TypeSyntax? returnType = null;
        var isAsync = false;
        string? returnOwnership = null;
        if (Match("returns"))
        {
            isAsync = Match("async");
            if (!isAsync)
                returnOwnership = MatchOwnership();
            returnType = ParseTypeExpression();
        }
        var items = ParseMemberBlockOrSemicolon();
        return new MethodDeclaration(name, parameters, hasExplicitSignature, returnType, isAsync, returnOwnership, items, location);
    }

    private PropertyDeclaration ParseProperty()
    {
        var location = Expect("property").Location;
        var first = ParseTypeExpression();
        TypeSyntax? type = first;
        string name;
        if (first is not TupleTypeSyntax && !CheckSymbol(";") && !BindingGrammar.IsPropertyModifier(Current.Text) && !CheckSymbol("{"))
            name = ExpectIdentifier("property name").Text;
        else
        {
            type = null;
            name = first is NamedTypeSyntax named && named.Arguments.Count == 0
                ? named.Name
                : throw Error("NWB1031", "A property needs a type and name.");
        }

        var readable = true;
        var writable = true;
        string? ownership = null;
        while (!CheckSymbol(";") && !CheckSymbol("{"))
        {
            switch (Current.Text)
            {
                case "readonly": readable = true; writable = false; Advance(); break;
                case "writeonly": readable = false; writable = true; Advance(); break;
                case "owned":
                case "borrowed":
                case "shared": ownership = Current.Text; Advance(); break;
                default: throw Error("NWB1010", $"Unexpected property modifier '{Current.Text}'.");
            }
        }
        var items = ParseMemberBlockOrSemicolon();
        foreach (var item in items)
        {
            switch (item)
            {
                case ReadableDeclaration readableDeclaration: readable = readableDeclaration.Value; break;
                case WritableDeclaration writableDeclaration: writable = writableDeclaration.Value; break;
                case OwnershipDeclaration ownershipDeclaration: ownership = ownershipDeclaration.Ownership; break;
            }
        }
        return new PropertyDeclaration(name, type, readable, writable, ownership, items, location);
    }

    private EventDeclaration ParseEvent()
    {
        var location = Expect("event").Location;
        var name = ExpectIdentifier("event name").Text;
        var items = ParseMemberBlockOrSemicolon();
        return new EventDeclaration(name, items, location);
    }

    private IReadOnlyList<MemberItem> ParseMemberBlockOrSemicolon()
    {
        if (Match(";")) return Array.Empty<MemberItem>();
        ExpectSymbol("{");
        var items = new List<MemberItem>();
        while (!CheckSymbol("}")) items.Add(ParseMemberItem());
        ExpectSymbol("}");
        return items;
    }

    private MemberItem ParseMemberItem() => Current.Text switch
    {
        "returns" => ParseReturn(),
        "parameter" => ParseParameterDeclaration(),
        "type" => ParseMemberType(),
        "execution" => ParseExecution(),
        "ownership" => ParseOwnership(),
        "callback" => ParseCallback(),
        "readable" => ParseBooleanMember((value, location) => new ReadableDeclaration(value, location)),
        "writable" => ParseBooleanMember((value, location) => new WritableDeclaration(value, location)),
        "managedName" => ParseNameMember(),
        "nativeName" => ParseNativeNameMember(),
        "adapter" => ParseAdapterMember(),
        _ => throw Error("NWB1010", $"Unexpected member option '{Current.Text}'."),
    };

    private ReturnDeclaration ParseReturn()
    {
        var location = Expect("returns").Location;
        var isAsync = Match("async");
        var ownership = isAsync ? null : MatchOwnership();
        var type = ParseTypeExpression();
        ExpectSymbol(";");
        return new ReturnDeclaration(type, isAsync, ownership, location);
    }

    private ParameterDeclaration ParseParameterDeclaration()
    {
        var location = Expect("parameter").Location;
        var name = ExpectIdentifier("parameter name").Text;
        ExpectSymbol(":");
        var parameter = ParseParameter(requireName: false, name);
        ExpectSymbol(";");
        return new ParameterDeclaration(parameter, location);
    }

    private MemberTypeDeclaration ParseMemberType()
    {
        var location = Expect("type").Location;
        var type = ParseTypeExpression();
        ExpectSymbol(";");
        return new MemberTypeDeclaration(type, location);
    }

    private ExecutionDeclaration ParseExecution()
    {
        var location = Expect("execution").Location;
        var policy = ExpectContextualValue("execution policy");
        if (!BindingGrammar.ExecutionPolicies.Contains(policy)) throw Error("NWB4021", $"Unknown execution policy '{policy}'.");
        ExpectSymbol(";");
        return new ExecutionDeclaration(policy, location);
    }

    private OwnershipDeclaration ParseOwnership()
    {
        var location = Expect("ownership").Location;
        var ownership = ExpectContextualValue("ownership kind");
        if (!BindingGrammar.OwnershipKinds.Contains(ownership)) throw Error("NWB4023", $"Unknown ownership kind '{ownership}'.");
        ExpectSymbol(";");
        return new OwnershipDeclaration(ownership, location);
    }

    private CallbackDeclaration ParseCallback()
    {
        var location = Expect("callback").Location;
        var policy = ExpectContextualValue("callback policy");
        if (!BindingGrammar.CallbackPolicies.Contains(policy)) throw Error("NWB4022", $"Unknown callback policy '{policy}'.");
        ExpectSymbol(";");
        return new CallbackDeclaration(policy, location);
    }

    private T ParseBooleanMember<T>(Func<bool, SourceLocation, T> factory) where T : MemberItem
    {
        var location = Current.Location;
        Advance();
        var value = Current.Kind switch
        {
            BindingTokenKind.True => true,
            BindingTokenKind.False => false,
            _ => throw Error("NWB1010", "Expected true or false."),
        };
        Advance();
        ExpectSymbol(";");
        return factory(value, location);
    }

    private ManagedNameDeclaration ParseNameMember()
    {
        var location = Expect("managedName").Location;
        var name = Expect(BindingTokenKind.String, "managed name").Text;
        ExpectSymbol(";");
        return new ManagedNameDeclaration(name, location);
    }

    private MemberNativeNameDeclaration ParseNativeNameMember()
    {
        var location = Expect("nativeName").Location;
        var name = Expect(BindingTokenKind.String, "native name").Text;
        ExpectSymbol(";");
        return new MemberNativeNameDeclaration(name, location);
    }

    private MemberAdapterDeclaration ParseAdapterMember()
    {
        var location = Expect("adapter").Location;
        var path = Expect(BindingTokenKind.String, "adapter path").Text;
        ExpectSymbol(";");
        return new MemberAdapterDeclaration(path, location);
    }

    private IReadOnlyList<ParameterSyntax> ParseParameters()
    {
        ExpectSymbol("(");
        var parameters = new List<ParameterSyntax>();
        if (!CheckSymbol(")"))
        {
            do parameters.Add(ParseParameter(requireName: false, null)); while (Match(","));
        }
        ExpectSymbol(")");
        return parameters;
    }

    private ParameterSyntax ParseParameter(bool requireName, string? forcedName)
    {
        var location = Current.Location;
        var passing = Current.Text is "ref" or "in" or "out" ? Current.Text : null;
        if (passing is not null) Advance();
        var type = ParseTypeExpression();
        string? name = forcedName;
        if (name is null && Check(BindingTokenKind.Identifier)) name = ExpectIdentifier("parameter name").Text;
        if (requireName && name is null) throw Error("NWB1010", "Parameter declarations require a name in a signature.");
        return new ParameterSyntax(passing, type, name, location);
    }

    private TypeSyntax ParseTypeExpression()
    {
        var location = Current.Location;
        TypeSyntax type;
        if (Match("("))
        {
            var elements = new List<TypeSyntax> { ParseTypeExpression() };
            while (Match(",")) elements.Add(ParseTypeExpression());
            if (elements.Count < 2) throw Error("NWB1032", "Tuple types require at least two elements.");
            ExpectSymbol(")");
            type = new TupleTypeSyntax(elements, location);
        }
        else
        {
            string? name = null;
            var token = Current;
            if (!token.IsEscapedIdentifier && BindingGrammar.CSharpAliases.TryGetValue(token.Text, out var replacement))
                throw Error("NWB1030", $"C# keyword alias '{token.Text}' is not a primitive type; use '{replacement}'.");

            if (!token.IsEscapedIdentifier && BindingGrammar.IsPrimitiveType(token.Text))
            {
                Advance();
                type = new PrimitiveTypeSyntax(token.Text, location);
            }
            else
            {
                name = ParseQualifiedName();
                type = new NamedTypeSyntax(name, Array.Empty<TypeSyntax>(), location);
            }

            if (Match("<"))
            {
                if (name is null)
                    throw Error("NWB1010", "Generic arguments require a named type.");

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

    private string? MatchOwnership()
    {
        if (Current.Text is "owned" or "borrowed" or "shared")
        {
            var value = Current.Text;
            Advance();
            return value;
        }
        return null;
    }

    private BindingToken Current => _tokens[_index];
    private bool Check(BindingTokenKind kind) => Current.Kind == kind;
    private bool CheckSymbol(string symbol) => Current.Kind == BindingTokenKind.Symbol && Current.Text == symbol;
    private bool Match(string text) { if (Current.Text != text) return false; Advance(); return true; }
    private BindingToken Advance() => _tokens[_index++];
    private BindingToken Expect(string text) { if (Current.Text != text) throw Error("NWB1010", $"Expected '{text}'."); return Advance(); }
    private BindingToken Expect(BindingTokenKind kind, string description) { if (Current.Kind != kind) throw Error("NWB1010", $"Expected {description}."); return Advance(); }
    private BindingToken ExpectIdentifier(string description)
    {
        var token = Expect(BindingTokenKind.Identifier, description);
        if (!token.IsEscapedIdentifier && BindingGrammar.ReservedWords.Contains(token.Text))
            throw new BindingParseException(new BindingDiagnostic("NWB1012", BindingDiagnosticSeverity.Error, $"'{token.Text}' is reserved here; use an @-escaped identifier.", token.Location));
        return token.IsEscapedIdentifier ? token with { Text = token.Text[1..] } : token;
    }
    private void ExpectSymbol(string symbol) => Expect(symbol);
    private BindingParseException Error(string id, string message) => new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, Current.Location));
}
