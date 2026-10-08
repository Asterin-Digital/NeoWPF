using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed class BindingModelBuilder
{
    public BindingModel Build(BindingDeclaration syntax)
    {
        var assembly = syntax.Items.OfType<AssemblyDeclaration>().SingleOrDefault();
        if (assembly is null)
            throw Error("NWB2010", "A binding must declare exactly one assembly.", syntax.Location);

        var types = syntax.Items
            .OfType<TypeDeclaration>()
            .Select(BuildType)
            .ToArray();

        return new BindingModel(
            syntax.Name,
            syntax.Items.OfType<VersionDeclaration>().SingleOrDefault()?.Value,
            syntax.Items.OfType<NativeNameDeclaration>().SingleOrDefault()?.Name,
            assembly.Path,
            syntax.Items.OfType<DependencyDeclaration>().Select(item => item.Path).ToArray(),
            syntax.Items.OfType<AdapterDeclaration>().Select(item => item.Path).ToArray(),
            types,
            syntax.Location);
    }

    private ManagedTypeModel BuildType(TypeDeclaration syntax)
    {
        var nativeName = syntax.Items.OfType<TypeNativeNameDeclaration>().SingleOrDefault()?.Name ?? LastName(syntax.Name);
        var constructors = syntax.Items.OfType<ConstructorDeclaration>().Select(BuildConstructor).ToArray();
        var methods = syntax.Items.OfType<MethodDeclaration>().Select(BuildMethod).ToArray();
        var properties = syntax.Items.OfType<PropertyDeclaration>().Select(BuildProperty).ToArray();
        var events = syntax.Items.OfType<EventDeclaration>().Select(BuildEvent).ToArray();

        return new ManagedTypeModel(
            syntax.Name,
            nativeName,
            syntax.Items.OfType<ConstructibleDeclaration>().Any(),
            constructors,
            methods,
            properties,
            events,
            syntax.Location);
    }

    private ManagedConstructorModel BuildConstructor(ConstructorDeclaration syntax) =>
        new(
            syntax.Parameters.Select(BuildParameter).ToArray(),
            GetExecution(syntax.Items),
            syntax.Location);

    private ManagedMethodModel BuildMethod(MethodDeclaration syntax)
    {
        var returnDeclaration = syntax.Items.OfType<ReturnDeclaration>().SingleOrDefault();
        var returnType = syntax.ReturnType is null
            ? returnDeclaration is null ? null : BuildType(returnDeclaration.Type)
            : BuildType(syntax.ReturnType);
        var isAsync = syntax.IsAsync || returnDeclaration?.IsAsync == true;
        var returnOwnership = syntax.ReturnOwnership ?? returnDeclaration?.Ownership;

        return new ManagedMethodModel(
            syntax.Items.OfType<ManagedNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Items.OfType<MemberNativeNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Parameters.Select(BuildParameter).ToArray(),
            syntax.HasExplicitSignature,
            returnType,
            isAsync,
            ParseOwnership(returnOwnership, syntax.Location),
            GetExecution(syntax.Items),
            syntax.Items.OfType<MemberAdapterDeclaration>().SingleOrDefault()?.Path,
            syntax.Location);
    }

    private ManagedPropertyModel BuildProperty(PropertyDeclaration syntax)
    {
        var type = syntax.Type;
        if (type is null)
            type = syntax.Items.OfType<MemberTypeDeclaration>().SingleOrDefault()?.Type;

        return new ManagedPropertyModel(
            syntax.Items.OfType<ManagedNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Items.OfType<MemberNativeNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            type is null ? null : BuildType(type),
            syntax.Readable,
            syntax.Writable,
            ParseOwnership(syntax.Ownership, syntax.Location),
            syntax.Location);
    }

    private ManagedEventModel BuildEvent(EventDeclaration syntax) =>
        new(
            syntax.Items.OfType<ManagedNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Items.OfType<MemberNativeNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            ParseCallback(syntax.Items.OfType<CallbackDeclaration>().SingleOrDefault()?.Policy, syntax.Location),
            syntax.Items.OfType<ParameterDeclaration>().Select(item => item.Parameter).Select(BuildParameter).ToArray(),
            syntax.Location);

    private ManagedParameterModel BuildParameter(ParameterSyntax syntax) =>
        new(
            syntax.Name,
            BuildType(syntax.Type),
            ParsePassingMode(syntax.PassingMode, syntax.Location),
            syntax.Location);

    private CanonicalType BuildType(TypeSyntax syntax) => syntax switch
    {
        PrimitiveTypeSyntax primitive => BuildPrimitive(primitive),
        NamedTypeSyntax named => BuildNamed(named),
        NullableTypeSyntax nullable => CanonicalType.Wrapped(CanonicalTypeKind.Nullable, BuildType(nullable.Element), nullable.Location),
        ArrayTypeSyntax array => CanonicalType.CreateArray(BuildType(array.Element), array.Rank, array.Location),
        TupleTypeSyntax tuple => CanonicalType.Tuple(tuple.Elements.Select(BuildType).ToArray(), tuple.Location),
        PointerTypeSyntax pointer => throw Error("NWB3010", "Pointer types are not supported in the first vertical slice.", pointer.Location),
        _ => throw Error("NWB3011", "The type shape is not supported in the first vertical slice.", syntax.Location),
    };

    private CanonicalType BuildPrimitive(PrimitiveTypeSyntax syntax) => syntax.Name switch
    {
        "void" => CanonicalType.Primitive(CanonicalTypeKind.Void, syntax.Name, IntegerWidth.None, syntax.Location),
        "bool" => CanonicalType.Primitive(CanonicalTypeKind.Boolean, syntax.Name, IntegerWidth.None, syntax.Location),
        "int8" => CanonicalType.Primitive(CanonicalTypeKind.SignedInteger, syntax.Name, IntegerWidth.Int8, syntax.Location),
        "int16" => CanonicalType.Primitive(CanonicalTypeKind.SignedInteger, syntax.Name, IntegerWidth.Int16, syntax.Location),
        "int32" => CanonicalType.Primitive(CanonicalTypeKind.SignedInteger, syntax.Name, IntegerWidth.Int32, syntax.Location),
        "int64" => CanonicalType.Primitive(CanonicalTypeKind.SignedInteger, syntax.Name, IntegerWidth.Int64, syntax.Location),
        "uint8" => CanonicalType.Primitive(CanonicalTypeKind.UnsignedInteger, syntax.Name, IntegerWidth.UInt8, syntax.Location),
        "uint16" => CanonicalType.Primitive(CanonicalTypeKind.UnsignedInteger, syntax.Name, IntegerWidth.UInt16, syntax.Location),
        "uint32" => CanonicalType.Primitive(CanonicalTypeKind.UnsignedInteger, syntax.Name, IntegerWidth.UInt32, syntax.Location),
        "uint64" => CanonicalType.Primitive(CanonicalTypeKind.UnsignedInteger, syntax.Name, IntegerWidth.UInt64, syntax.Location),
        "float" or "double" => CanonicalType.Primitive(CanonicalTypeKind.FloatingPoint, syntax.Name, IntegerWidth.None, syntax.Location),
        "decimal" => CanonicalType.Primitive(CanonicalTypeKind.Decimal, syntax.Name, IntegerWidth.None, syntax.Location),
        "string" => CanonicalType.Primitive(CanonicalTypeKind.String, syntax.Name, IntegerWidth.None, syntax.Location),
        "bytes" => CanonicalType.Primitive(CanonicalTypeKind.Bytes, syntax.Name, IntegerWidth.None, syntax.Location),
        _ => throw Error("NWB3012", $"Unknown primitive type '{syntax.Name}'.", syntax.Location),
    };

    private CanonicalType BuildNamed(NamedTypeSyntax syntax)
    {
        var arguments = syntax.Arguments.Select(BuildType).ToArray();
        var kind = syntax.Arguments.Count > 0 ? CanonicalTypeKind.Generic : CanonicalTypeKind.NamedClass;
        if (syntax.Name == "System.Threading.Tasks.Task") kind = CanonicalTypeKind.Task;
        if (syntax.Name == "System.Threading.Tasks.ValueTask") kind = CanonicalTypeKind.ValueTask;
        return CanonicalType.Named(kind, syntax.Name, arguments, null, syntax.Location);
    }

    private static ManagedExecutionPolicy GetExecution(IEnumerable<MemberItem> items)
    {
        var value = items.OfType<ExecutionDeclaration>().SingleOrDefault()?.Policy;
        return value switch
        {
            null => ManagedExecutionPolicy.Unspecified,
            "Managed" => ManagedExecutionPolicy.Managed,
            "Calling" => ManagedExecutionPolicy.Calling,
            "Application" => ManagedExecutionPolicy.Application,
            "Background" => ManagedExecutionPolicy.Background,
            _ => throw new InvalidOperationException($"Unknown execution policy '{value}'."),
        };
    }

    private static ManagedOwnership? ParseOwnership(string? value, SourceLocation location) => value switch
    {
        null => null,
        "owned" => ManagedOwnership.Owned,
        "borrowed" => ManagedOwnership.Borrowed,
        "shared" => ManagedOwnership.Shared,
        _ => throw Error("NWB4023", $"Unknown ownership kind '{value}'.", location),
    };

    private static ManagedCallbackPolicy ParseCallback(string? value, SourceLocation location) => value switch
    {
        null => ManagedCallbackPolicy.Application,
        "Managed" => ManagedCallbackPolicy.Managed,
        "Application" => ManagedCallbackPolicy.Application,
        "Background" => ManagedCallbackPolicy.Background,
        _ => throw Error("NWB4022", $"Unknown callback policy '{value}'.", location),
    };

    private static ParameterPassingMode ParsePassingMode(string? value, SourceLocation location) => value switch
    {
        null => ParameterPassingMode.Value,
        "ref" => ParameterPassingMode.Ref,
        "in" => ParameterPassingMode.In,
        "out" => ParameterPassingMode.Out,
        _ => throw Error("NWB4024", $"Unknown parameter passing mode '{value}'.", location),
    };

    private static string LastName(string name) => name[(name.LastIndexOf('.') + 1)..];

    private static BindingParseException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
