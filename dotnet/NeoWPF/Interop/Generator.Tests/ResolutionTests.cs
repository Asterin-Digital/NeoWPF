using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class ResolutionTests
{
    [Fact]
    public void ClassifiesEnumsStructsDelegatesAndNestedTypes()
    {
        var assembly = ReadAssembly();

        var state = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceState");
        var money = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.Money");
        var callback = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceCallback");
        var nested = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.NestedContainer+NestedResult");

        Assert.True(state.IsEnum);
        Assert.True(money.IsStruct);
        Assert.True(callback.IsDelegate);
        Assert.True(nested.IsClass);
    }

    [Fact]
    public void DiscoversTypesInStableOrdinalOrder()
    {
        var assembly = ReadAssembly();
        var names = assembly.Types.Select(type => type.FullName).ToArray();

        Assert.Equal(names.OrderBy(name => name, StringComparer.Ordinal), names);
    }

    [Fact]
    public void ResolvesStaticMembersOnlyWhenStaticStateMatches()
    {
        var reader = new AssemblyReader();
        var service = reader.ResolveType(ReadAssembly(), "BasicAssembly.Domain.InvoiceService");

        var method = reader.ResolveMethod(service, "CreateRequest", ["System.Decimal"], isStatic: true);
        var exception = Assert.Throws<BindingParseException>(() => reader.ResolveMethod(service, "CreateRequest", ["System.Decimal"]));

        Assert.True(method.IsStatic);
        Assert.Equal("NWB2012", exception.Diagnostic.Id);
    }

    [Fact]
    public void DistinguishesMissingMembersFromSignatureMismatches()
    {
        var reader = new AssemblyReader();
        var service = reader.ResolveType(ReadAssembly(), "BasicAssembly.Domain.InvoiceService");

        var missing = Assert.Throws<BindingParseException>(() => reader.ResolveMethod(service, "DoesNotExist", []));
        var mismatch = Assert.Throws<BindingParseException>(() => reader.ResolveMethod(service, "Calculate", ["System.Int64"]));

        Assert.Equal("NWB2012", missing.Diagnostic.Id);
        Assert.Equal("NWB2014", mismatch.Diagnostic.Id);
    }

    [Fact]
    public void RetainsRefInAndOutParameterMetadata()
    {
        var service = new AssemblyReader().ResolveType(ReadAssembly(), "BasicAssembly.Domain.InvoiceService");
        var tryGet = Assert.Single(service.Methods, method => method.Name == "TryGetMoney");
        var update = Assert.Single(service.Methods, method => method.Name == "UpdateMoney");

        Assert.Collection(
            tryGet.Parameters,
            parameter => Assert.False(parameter.IsByReference),
            parameter =>
            {
                Assert.True(parameter.IsByReference);
                Assert.True(parameter.IsOut);
                Assert.False(parameter.IsIn);
            });
        Assert.Collection(
            update.Parameters,
            parameter =>
            {
                Assert.True(parameter.IsByReference);
                Assert.False(parameter.IsOut);
                Assert.False(parameter.IsIn);
            },
            parameter =>
            {
                Assert.True(parameter.IsByReference);
                Assert.False(parameter.IsOut);
                Assert.True(parameter.IsIn);
            });
    }

    [Fact]
    public void ReportsGenericTypeAndMethodArity()
    {
        var assembly = ReadAssembly();
        var genericMethod = Assert.Single(
            Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService").Methods,
            method => method.Name == "Echo");

        Assert.Equal(1, genericMethod.GenericArity);
    }

    private static InspectedAssembly ReadAssembly() =>
        new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
}
