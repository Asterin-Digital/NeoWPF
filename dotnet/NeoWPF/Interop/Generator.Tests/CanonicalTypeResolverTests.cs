using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class CanonicalTypeResolverTests
{
    private static readonly SourceLocation Location = new("resolver.nwbind", 1, 1);

    [Fact]
    public void ResolvesFixtureTypesWithAssemblyIdentityAndKind()
    {
        var assembly = ReadAssembly();
        var resolver = new CanonicalTypeResolver();

        var classType = resolver.Resolve(Named("BasicAssembly.Domain.InvoiceService"), assembly);
        var structType = resolver.Resolve(Named("BasicAssembly.Domain.Money"), assembly);
        var enumType = resolver.Resolve(Named("BasicAssembly.Domain.InvoiceState"), assembly);
        var delegateType = resolver.Resolve(Named("BasicAssembly.Domain.InvoiceCallback"), assembly);

        Assert.Equal(CanonicalTypeKind.NamedClass, classType.Kind);
        Assert.Equal(CanonicalTypeKind.NamedStruct, structType.Kind);
        Assert.Equal(CanonicalTypeKind.Enum, enumType.Kind);
        Assert.Equal(CanonicalTypeKind.Delegate, delegateType.Kind);
        Assert.Equal("BasicAssembly", classType.AssemblyIdentity);
    }

    [Fact]
    public void ResolvesRecursiveArraysNullableAndTaskShapes()
    {
        var assembly = ReadAssembly();
        var resolver = new CanonicalTypeResolver();
        var request = Named("BasicAssembly.Domain.InvoiceRequest");
        var array = CanonicalType.CreateArray(
            CanonicalType.Wrapped(CanonicalTypeKind.Nullable, request, Location),
            1,
            Location);
        var task = CanonicalType.Named(
            CanonicalTypeKind.Task,
            "System.Threading.Tasks.Task",
            [CanonicalType.Primitive(CanonicalTypeKind.Decimal, "decimal", IntegerWidth.None, Location)],
            null,
            Location);

        var resolvedArray = resolver.Resolve(array, assembly);
        var resolvedTask = resolver.Resolve(task, assembly);

        Assert.Equal(CanonicalTypeKind.Array, resolvedArray.Kind);
        Assert.Equal(CanonicalTypeKind.Nullable, resolvedArray.Element!.Kind);
        Assert.Equal("BasicAssembly", resolvedArray.Element.Element!.AssemblyIdentity);
        Assert.Equal(CanonicalTypeKind.Task, resolvedTask.Kind);
        Assert.Equal(CanonicalTypeKind.Decimal, resolvedTask.Arguments[0].Kind);
    }

    [Fact]
    public void ReportsMissingAndUnsupportedCanonicalTypes()
    {
        var assembly = ReadAssembly();
        var resolver = new CanonicalTypeResolver();

        var missing = Assert.Throws<BindingParseException>(() => resolver.Resolve(Named("BasicAssembly.Domain.Missing"), assembly));
        var pointer = Assert.Throws<BindingParseException>(() => resolver.Resolve(
            new CanonicalType(CanonicalTypeKind.Pointer, "pointer", IntegerWidth.None, [], null, 0, null, Location),
            assembly));

        Assert.Equal("NWB2011", missing.Diagnostic.Id);
        Assert.Equal("NWB3010", pointer.Diagnostic.Id);
    }

    private static CanonicalType Named(string name) =>
        CanonicalType.Named(CanonicalTypeKind.NamedClass, name, [], null, Location);

    private static InspectedAssembly ReadAssembly() =>
        new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
}
