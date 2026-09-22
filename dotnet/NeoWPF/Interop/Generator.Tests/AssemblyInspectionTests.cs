using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class AssemblyInspectionTests
{
    [Fact]
    public void ReadsFixtureIdentityAndPublicTypes()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);

        Assert.Equal("BasicAssembly", assembly.Identity.Name);
        Assert.NotEqual(Guid.Empty, assembly.Identity.ModuleVersionId);
        Assert.Contains(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService");
        Assert.Contains(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceRequest");
        Assert.Contains(assembly.Types, type => type.FullName == "BasicAssembly.Domain.NestedContainer+NestedResult");
    }

    [Fact]
    public void ReadsConstructorsPropertiesEventsAndOverloads()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var service = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService");

        Assert.Contains(service.Methods, method => method.IsConstructor &&
            method.Parameters.Count == 1 &&
            method.Parameters[0].TypeName == "System.String");
        Assert.Contains(service.Properties, property => property.Name == "AccountName" && property.CanRead && !property.CanWrite);
        Assert.Contains(service.Events, @event => @event.Name == "StateChanged");
        Assert.Equal(2, service.Methods.Count(method => method.Name == "Calculate"));
    }

    [Fact]
    public void PreservesAsyncAndCancellationTokenSignatures()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var service = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService");
        var method = Assert.Single(service.Methods, candidate => candidate.Name == "CalculateAsync");

        Assert.StartsWith("System.Threading.Tasks.Task`1[[System.Decimal, System.Private.CoreLib", method.ReturnTypeName, StringComparison.Ordinal);
        Assert.Equal(2, method.Parameters.Count);
        Assert.Equal("System.Threading.CancellationToken", method.Parameters[1].TypeName);
    }

    [Fact]
    public void ResolvesMethodsByExactSignature()
    {
        var reader = new AssemblyReader();
        var assembly = reader.Read(typeof(InvoiceService).Assembly.Location);
        var service = reader.ResolveType(assembly, "BasicAssembly.Domain.InvoiceService");

        var method = reader.ResolveMethod(service, "Calculate", ["BasicAssembly.Domain.InvoiceRequest"]);

        Assert.Equal("System.Decimal", method.ReturnTypeName);
    }

    [Fact]
    public void ReportsMissingTypesAndSignatureMismatches()
    {
        var reader = new AssemblyReader();
        var assembly = reader.Read(typeof(InvoiceService).Assembly.Location);
        var service = reader.ResolveType(assembly, "BasicAssembly.Domain.InvoiceService");

        var missingType = Assert.Throws<BindingParseException>(() => reader.ResolveType(assembly, "BasicAssembly.Missing"));
        var mismatch = Assert.Throws<BindingParseException>(() => reader.ResolveMethod(service, "Calculate", ["System.String"]));

        Assert.Equal("NWB2011", missingType.Diagnostic.Id);
        Assert.Equal("NWB2014", mismatch.Diagnostic.Id);
    }

    [Fact]
    public void InspectsGenericStaticArrayAndByReferenceMembers()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var service = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService");

        var generic = Assert.Single(service.Methods, method => method.Name == "Echo");
        var factory = Assert.Single(service.Methods, method => method.Name == "CreateRequest");
        var sum = Assert.Single(service.Methods, method => method.Name == "Sum");
        var tryGet = Assert.Single(service.Methods, method => method.Name == "TryGetMoney");
        var update = Assert.Single(service.Methods, method => method.Name == "UpdateMoney");

        Assert.Equal(1, generic.GenericArity);
        Assert.True(factory.IsStatic);
        Assert.Equal("System.Decimal[]", sum.Parameters[0].TypeName);
        Assert.True(tryGet.Parameters[1].IsByReference);
        Assert.True(tryGet.Parameters[1].IsOut);
        Assert.True(update.Parameters[0].IsByReference);
        Assert.True(update.Parameters[1].IsByReference);
        Assert.True(update.Parameters[1].IsIn);
    }
}
