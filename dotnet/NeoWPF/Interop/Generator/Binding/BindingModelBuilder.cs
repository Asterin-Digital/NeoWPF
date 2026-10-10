using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed class BindingModelBuilder
{
    public BindingModel Build(BindingDeclaration syntax)
    {
        var assembly = syntax.Items.OfType<AssemblyDeclaration>().SingleOrDefault();
        if (assembly is null)
            throw Error(BindingDiagnosticIds.MissingAssembly, "A binding must declare exactly one assembly.", syntax.Location);

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
            returnOwnership,
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
            syntax.Ownership,
            syntax.Location);
    }

    private ManagedEventModel BuildEvent(EventDeclaration syntax) =>
        new(
            syntax.Items.OfType<ManagedNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Items.OfType<MemberNativeNameDeclaration>().SingleOrDefault()?.Name ?? syntax.Name,
            syntax.Items.OfType<CallbackDeclaration>().SingleOrDefault()?.Policy ?? ManagedCallbackPolicy.Application,
            syntax.Items.OfType<ParameterDeclaration>().Select(item => item.Parameter).Select(BuildParameter).ToArray(),
            syntax.Location);

    private ManagedParameterModel BuildParameter(ParameterSyntax syntax) =>
        new(
            syntax.Name,
            BuildType(syntax.Type),
            syntax.PassingMode,
            syntax.Location);

    private CanonicalType BuildType(TypeSyntax syntax) => syntax switch
    {
        NamedTypeSyntax named => BuildNamed(named),
        NullableTypeSyntax nullable => CanonicalType.Wrapped(CanonicalTypeKind.Nullable, BuildType(nullable.Element), nullable.Location),
        ArrayTypeSyntax array => CanonicalType.CreateArray(BuildType(array.Element), array.Rank, array.Location),
        TupleTypeSyntax tuple => CanonicalType.Tuple(tuple.Elements.Select(BuildType).ToArray(), tuple.Location),
        PointerTypeSyntax pointer => throw Error(BindingDiagnosticIds.UnsupportedType, "Pointer types are not supported in the first vertical slice.", pointer.Location),
        _ => throw Error(BindingDiagnosticIds.UnresolvedTypeShape, "The type shape is not supported in the first vertical slice.", syntax.Location),
    };

    private CanonicalType BuildNamed(NamedTypeSyntax syntax)
    {
        if (syntax.Arguments.Count == 0 && PrimitiveTypeRegistry.TryGet(syntax.Name, out var primitive))
        {
            return CanonicalType.Primitive(
                primitive.Kind,
                primitive.Name,
                primitive.Width,
                syntax.Location);
        }

        var arguments = syntax.Arguments.Select(BuildType).ToArray();
        var kind = syntax.Arguments.Count > 0 ? CanonicalTypeKind.Generic : CanonicalTypeKind.NamedClass;
        if (syntax.Name == "System.Threading.Tasks.Task") kind = CanonicalTypeKind.Task;
        if (syntax.Name == "System.Threading.Tasks.ValueTask") kind = CanonicalTypeKind.ValueTask;
        return CanonicalType.Named(kind, syntax.Name, arguments, null, syntax.Location);
    }

    private static ManagedExecutionPolicy GetExecution(IEnumerable<MemberItem> items)
    {
        return items.OfType<ExecutionDeclaration>().SingleOrDefault()?.Policy ??
            ManagedExecutionPolicy.Unspecified;
    }

    private static string LastName(string name) => name[(name.LastIndexOf('.') + 1)..];

    private static BindingDiagnosticException Error(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
