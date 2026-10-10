using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Parsing;

public sealed partial class BindingParser
{
    private ConstructorDeclaration ParseConstructor()
    {
        var location = Expect("constructor").Location;
        var parameters = ParseParameters();
        var items = ParseMemberBlockOrSemicolon(MemberKind.Constructor);
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
        ManagedOwnership? returnOwnership = null;
        if (Match("returns"))
        {
            isAsync = Match("async");
            if (!isAsync)
                returnOwnership = MatchOwnership();
            returnType = ParseTypeExpression();
        }

        var items = ParseMemberBlockOrSemicolon(MemberKind.Method);
        var blockReturn = items.OfType<ReturnDeclaration>().FirstOrDefault();
        if (returnType is not null && blockReturn is not null)
            throw Error(BindingDiagnosticIds.DuplicateMemberOption, "The method return option was declared more than once.", blockReturn.Location);

        return new MethodDeclaration(name, parameters, hasExplicitSignature, returnType, isAsync, returnOwnership, items, location);
    }

    private PropertyDeclaration ParseProperty()
    {
        var location = Expect("property").Location;
        var first = ParseTypeExpression();
        TypeSyntax? type = first;
        string name;
        if (first is not TupleTypeSyntax && !CheckSymbol(";") && !IsPropertyModifier() && !CheckSymbol("{"))
        {
            name = ExpectIdentifier("property name").Text;
        }
        else
        {
            type = null;
            name = first is NamedTypeSyntax named && named.Arguments.Count == 0 && !named.Name.Contains('.') &&
                !BindingGrammar.IsPrimitiveTypeName(named.Name)
                ? named.Name
                : throw Error(BindingDiagnosticIds.InvalidPropertyDeclaration, "A property needs an unqualified name, and an optional type.");
        }

        var readable = true;
        var writable = true;
        ManagedOwnership? ownership = null;
        var inlineOptions = new HashSet<string>(StringComparer.Ordinal);
        while (!CheckSymbol(";") && !CheckSymbol("{") && !Check(BindingTokenKind.End))
        {
            string optionName;
            if (IsKeyword("readonly"))
            {
                optionName = "access";
                readable = true;
                writable = false;
            }
            else if (IsKeyword("writeonly"))
            {
                optionName = "access";
                readable = false;
                writable = true;
            }
            else if (IsOwnershipKeyword())
            {
                optionName = "ownership";
                ownership = ParseOwnershipValue(Current.Text, Current.Location);
            }
            else
            {
                throw Error(BindingDiagnosticIds.UnexpectedToken, $"Unexpected property modifier '{Current.Text}'.");
            }

            if (!inlineOptions.Add(optionName))
                throw Error(BindingDiagnosticIds.DuplicateMemberOption, $"The '{optionName}' property option was declared more than once.");
            Advance();
        }

        var items = ParseMemberBlockOrSemicolon(MemberKind.Property);
        foreach (var item in items)
        {
            switch (item)
            {
                case ReadableDeclaration readableDeclaration:
                    if (inlineOptions.Contains("access"))
                        throw Error(BindingDiagnosticIds.DuplicateMemberOption, "The property access option was declared more than once.", item.Location);
                    readable = readableDeclaration.Value;
                    break;
                case WritableDeclaration writableDeclaration:
                    if (inlineOptions.Contains("access"))
                        throw Error(BindingDiagnosticIds.DuplicateMemberOption, "The property access option was declared more than once.", item.Location);
                    writable = writableDeclaration.Value;
                    break;
                case OwnershipDeclaration ownershipDeclaration:
                    if (inlineOptions.Contains("ownership"))
                        throw Error(BindingDiagnosticIds.DuplicateMemberOption, "The property ownership option was declared more than once.", item.Location);
                    ownership = ownershipDeclaration.Ownership;
                    break;
            }
        }

        var memberType = items.OfType<MemberTypeDeclaration>().FirstOrDefault();
        if (type is not null && memberType is not null)
            throw Error(BindingDiagnosticIds.DuplicateMemberOption, "The property type was declared more than once.", memberType.Location);
        if (type is null)
            type = memberType?.Type;

        return new PropertyDeclaration(name, type, readable, writable, ownership, items, location);
    }

    private EventDeclaration ParseEvent()
    {
        var location = Expect("event").Location;
        var name = ExpectIdentifier("event name").Text;
        var items = ParseMemberBlockOrSemicolon(MemberKind.Event);
        return new EventDeclaration(name, items, location);
    }

    private IReadOnlyList<MemberItem> ParseMemberBlockOrSemicolon(MemberKind kind)
    {
        if (Match(";")) return Array.Empty<MemberItem>();
        ExpectSymbol("{");
        var items = new List<MemberItem>();
        var seenOptions = new HashSet<string>(StringComparer.Ordinal);
        while (!CheckSymbol("}") && !Check(BindingTokenKind.End))
        {
            var item = ParseMemberItem();
            if (!IsAllowedMemberItem(kind, item))
            {
                throw Error(
                    BindingDiagnosticIds.InvalidMemberOption,
                    $"The '{GetMemberOptionName(item)}' option is not valid on a {kind.ToString().ToLowerInvariant()}.",
                    item.Location);
            }

            var optionName = GetUniqueMemberOptionName(item);
            if (optionName is not null && !seenOptions.Add(optionName))
            {
                throw Error(
                    BindingDiagnosticIds.DuplicateMemberOption,
                    $"The '{optionName}' option was declared more than once.",
                    item.Location);
            }
            items.Add(item);
        }
        ExpectSymbol("}");
        return items;
    }

    private static bool IsAllowedMemberItem(MemberKind kind, MemberItem item) => kind switch
    {
        MemberKind.Constructor => item is ExecutionDeclaration or MemberAdapterDeclaration,
        MemberKind.Method => item is ReturnDeclaration or ExecutionDeclaration or MemberAdapterDeclaration or
            ManagedNameDeclaration or MemberNativeNameDeclaration,
        MemberKind.Property => item is MemberTypeDeclaration or OwnershipDeclaration or ReadableDeclaration or
            WritableDeclaration or ManagedNameDeclaration or MemberNativeNameDeclaration or MemberAdapterDeclaration,
        MemberKind.Event => item is ParameterDeclaration or CallbackDeclaration or ManagedNameDeclaration or
            MemberNativeNameDeclaration or MemberAdapterDeclaration,
        _ => false,
    };

    private static string GetMemberOptionName(MemberItem item) => item switch
    {
        ReturnDeclaration => "returns",
        ParameterDeclaration => "parameter",
        MemberTypeDeclaration => "type",
        ExecutionDeclaration => "execution",
        OwnershipDeclaration => "ownership",
        CallbackDeclaration => "callback",
        ReadableDeclaration => "readable",
        WritableDeclaration => "writable",
        ManagedNameDeclaration => "managedName",
        MemberNativeNameDeclaration => "nativeName",
        MemberAdapterDeclaration => "adapter",
        _ => "unknown",
    };

    private static string? GetUniqueMemberOptionName(MemberItem item) => item is ParameterDeclaration
        ? null
        : GetMemberOptionName(item);

    private static void CheckDuplicate<T>(IEnumerable<T> values, string optionName)
        where T : BindingNode
    {
        var duplicate = values.Skip(1).FirstOrDefault();
        if (duplicate is not null)
        {
            throw new BindingDiagnosticException(new BindingDiagnostic(
                BindingDiagnosticIds.DuplicateBindingOrTypeOption,
                BindingDiagnosticSeverity.Error,
                $"The '{optionName}' option was declared more than once.",
                duplicate.Location));
        }
    }

    private MemberItem ParseMemberItem()
    {
        if (IsKeyword("returns")) return ParseReturn();
        if (IsKeyword("parameter")) return ParseParameterDeclaration();
        if (IsKeyword("type")) return ParseMemberType();
        if (IsKeyword("execution")) return ParseExecution();
        if (IsKeyword("ownership")) return ParseOwnership();
        if (IsKeyword("callback")) return ParseCallback();
        if (IsKeyword("readable")) return ParseBooleanMember((value, location) => new ReadableDeclaration(value, location));
        if (IsKeyword("writable")) return ParseBooleanMember((value, location) => new WritableDeclaration(value, location));
        if (IsKeyword("managedName")) return ParseNameMember();
        if (IsKeyword("nativeName")) return ParseNativeNameMember();
        if (IsKeyword("adapter")) return ParseAdapterMember();
        throw Error(BindingDiagnosticIds.UnexpectedToken, $"Unexpected member option '{Current.Text}'.");
    }

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
        var policyName = ExpectContextualValue("execution policy");
        var policy = policyName switch
        {
            "Managed" => ManagedExecutionPolicy.Managed,
            "Calling" => ManagedExecutionPolicy.Calling,
            "Application" => ManagedExecutionPolicy.Application,
            "Background" => ManagedExecutionPolicy.Background,
            _ => throw Error(BindingDiagnosticIds.UnknownExecutionPolicy, $"Unknown execution policy '{policyName}'."),
        };
        ExpectSymbol(";");
        return new ExecutionDeclaration(policy, location);
    }

    private OwnershipDeclaration ParseOwnership()
    {
        var location = Expect("ownership").Location;
        var ownershipName = ExpectContextualValue("ownership kind");
        var ownership = ParseOwnershipValue(ownershipName, location);
        ExpectSymbol(";");
        return new OwnershipDeclaration(ownership, location);
    }

    private CallbackDeclaration ParseCallback()
    {
        var location = Expect("callback").Location;
        var policyName = ExpectContextualValue("callback policy");
        var policy = policyName switch
        {
            "Managed" => ManagedCallbackPolicy.Managed,
            "Application" => ManagedCallbackPolicy.Application,
            "Background" => ManagedCallbackPolicy.Background,
            _ => throw Error(BindingDiagnosticIds.UnknownCallbackPolicy, $"Unknown callback policy '{policyName}'."),
        };
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
            _ => throw Error(BindingDiagnosticIds.UnexpectedToken, "Expected true or false."),
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
        var passing = ParameterPassingMode.Value;
        if (IsKeyword("ref"))
        {
            passing = ParameterPassingMode.Ref;
            Advance();
        }
        else if (IsKeyword("in"))
        {
            passing = ParameterPassingMode.In;
            Advance();
        }
        else if (IsKeyword("out"))
        {
            passing = ParameterPassingMode.Out;
            Advance();
        }
        var type = ParseTypeExpression();
        string? name = forcedName;
        if (name is null && Check(BindingTokenKind.Identifier)) name = ExpectIdentifier("parameter name").Text;
        if (requireName && name is null)
            throw Error(BindingDiagnosticIds.UnexpectedToken, "Parameter declarations require a name in a signature.");
        return new ParameterSyntax(passing, type, name, location);
    }

    private static ManagedOwnership ParseOwnershipValue(string value, SourceLocation location) => value switch
    {
        "owned" => ManagedOwnership.Owned,
        "borrowed" => ManagedOwnership.Borrowed,
        "shared" => ManagedOwnership.Shared,
        _ => throw Error(BindingDiagnosticIds.UnknownOwnership, $"Unknown ownership kind '{value}'.", location),
    };
}