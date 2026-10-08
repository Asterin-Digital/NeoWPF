using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
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
    public void ResolvesEachOverloadByItsExactParameterType()
    {
        var reader = new AssemblyReader();
        var service = reader.ResolveType(ReadAssembly(), "BasicAssembly.Domain.InvoiceService");

        var requestOverload = reader.ResolveMethod(
            service,
            "Calculate",
            ["BasicAssembly.Domain.InvoiceRequest"]);
        var integerOverload = reader.ResolveMethod(service, "Calculate", ["System.Int32"]);

        Assert.Equal("System.Decimal", requestOverload.ReturnTypeName);
        Assert.Equal("System.Decimal", integerOverload.ReturnTypeName);
        Assert.Equal("BasicAssembly.Domain.InvoiceRequest", requestOverload.Parameters[0].TypeName);
        Assert.Equal("System.Int32", integerOverload.Parameters[0].TypeName);
    }

    [Fact]
    public void RejectsAmbiguousStaticStateWhenTheBindingDoesNotSpecifyIt()
    {
        var location = new SourceLocation("resolver.nwbind", 1, 1);
        var parameter = new InspectedParameter("value", "System.Int32", false, false, false);
        var method = new InspectedMethod("Convert", "System.String", [parameter], false, false, 0);
        var staticMethod = method with { IsStatic = true };
        var type = new InspectedType(
            "Acme.Converter",
            true,
            false,
            false,
            false,
            0,
            [method, staticMethod],
            [],
            []);

        var exception = Assert.Throws<BindingParseException>(() => new AssemblyReader().ResolveMethod(
            type,
            "Convert",
            ["System.Int32"],
            isStatic: null));

        Assert.Equal("NWB2013", exception.Diagnostic.Id);
    }

    [Fact]
    public void ResolvesNameOnlyMethodsOnlyWhenTheCandidateIsUnique()
    {
        var reader = new AssemblyReader();
        var service = reader.ResolveType(ReadAssembly(), "BasicAssembly.Domain.InvoiceService");

        var unique = reader.ResolveMethodByName(service, "CreateRequest");
        var ambiguous = Assert.Throws<BindingParseException>(() => reader.ResolveMethodByName(service, "Calculate"));

        Assert.Equal("CreateRequest", unique.Name);
        Assert.Equal("NWB2013", ambiguous.Diagnostic.Id);
        Assert.Contains("Calculate(BasicAssembly.Domain.InvoiceRequest)", ambiguous.Diagnostic.Message);
        Assert.Contains("Calculate(System.Int32)", ambiguous.Diagnostic.Message);
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
