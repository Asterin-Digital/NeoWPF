using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using PluginAssembly;
using SecondaryDependency;
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
    public void ResolvesFrameworkTypesFromTheRuntimeClosure()
    {
        var reader = new AssemblyReader();
        var assembly = reader.Read(typeof(InvoiceService).Assembly.Location);
        var location = new SourceLocation("framework.nwbind", 3, 12);

        var type = reader.ResolveType(assembly, "System.Threading.CancellationToken", location);

        Assert.Equal("System.Threading.CancellationToken", type.FullName);
        Assert.Equal("System.Private.CoreLib", type.AssemblyName);
    }

    [Fact]
    public void ResolvesTypesFromDeclaredDependencyAssemblies()
    {
        var reader = new AssemblyReader();
        var pluginPath = typeof(PluginService).Assembly.Location;
        var dependencyPath = typeof(Calculator).Assembly.Location;
        var assembly = reader.Read(pluginPath, [dependencyPath]);

        var type = reader.ResolveType(
            assembly,
            "SecondaryDependency.Calculator",
            new SourceLocation("plugin.nwbind", 4, 9));

        Assert.Equal("SecondaryDependency", type.AssemblyName);
        Assert.Contains(assembly.Dependencies, dependency => dependency.Name == "SecondaryDependency");
    }

    [Fact]
    public void ClassifiesClrTypeShapes()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var interfaceType = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.IInvoiceCalculator");
        var abstractType = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.AbstractInvoiceService");
        var staticType = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceMath");
        var byRefLikeType = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceSpan");

        Assert.True(interfaceType.IsInterface);
        Assert.True(interfaceType.IsVisible);
        Assert.True(abstractType.IsAbstract);
        Assert.True(staticType.IsStatic);
        Assert.True(byRefLikeType.IsByRefLike);
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
        Assert.DoesNotContain(service.Methods, method => method.Name is "get_Item" or "set_Item");
        var indexer = Assert.Single(service.Properties, property => property.Name == "Item");
        Assert.Equal("System.Int32", Assert.Single(indexer.IndexParameters).TypeName);
    }

    [Fact]
    public void PreservesAsyncAndCancellationTokenSignatures()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var service = Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService");
        var method = Assert.Single(service.Methods, candidate => candidate.Name == "CalculateAsync");

        Assert.Equal("System.Threading.Tasks.Task<System.Decimal>", method.ReturnTypeName);
        Assert.Equal(1, method.ReturnType.GenericArity);
        Assert.Equal("System.Decimal", Assert.Single(method.ReturnType.Arguments).DisplayName);
        Assert.Equal(2, method.Parameters.Count);
        Assert.Equal("System.Threading.CancellationToken", method.Parameters[1].TypeName);
    }

    [Fact]
    public void MatchesAsyncBindingResultAgainstTaskResultArgument()
    {
        var assembly = new AssemblyReader().Read(typeof(InvoiceService).Assembly.Location);
        var method = Assert.Single(
            Assert.Single(assembly.Types, type => type.FullName == "BasicAssembly.Domain.InvoiceService").Methods,
            candidate => candidate.Name == "CalculateAsync");
        var decimalType = CanonicalType.Primitive(CanonicalTypeKind.Decimal, "decimal", IntegerWidth.None, new SourceLocation("async.nwbind", 2, 30));
        var integerType = CanonicalType.Primitive(CanonicalTypeKind.SignedInteger, "int32", IntegerWidth.Int32, decimalType.Location);

        Assert.True(AssemblyReader.AsyncResultMatches(decimalType, method.ReturnType));
        Assert.False(AssemblyReader.AsyncResultMatches(integerType, method.ReturnType));
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

        var missingType = Assert.Throws<BindingDiagnosticException>(() => reader.ResolveType(assembly, "BasicAssembly.Missing"));
        var mismatch = Assert.Throws<BindingDiagnosticException>(() => reader.ResolveMethod(service, "Calculate", ["System.String"]));

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

    [Fact]
    public void ResolvesMethodsByPassingModeAndGenericArity()
    {
        var reader = new AssemblyReader();
        var service = reader.ResolveType(
            reader.Read(typeof(InvoiceService).Assembly.Location),
            "BasicAssembly.Domain.InvoiceService");

        var tryGet = reader.ResolveMethod(
            service,
            "TryGetMoney",
            ["System.Decimal", "BasicAssembly.Domain.Money"],
            parameterPassingModes: [ParameterPassingMode.Value, ParameterPassingMode.Out]);
        var update = reader.ResolveMethod(
            service,
            "UpdateMoney",
            ["BasicAssembly.Domain.Money", "System.Decimal"],
            parameterPassingModes: [ParameterPassingMode.Ref, ParameterPassingMode.In]);
        var echo = reader.ResolveMethod(
            service,
            "Echo",
            ["T"],
            genericArity: 1);

        Assert.True(tryGet.Parameters[1].IsOut);
        Assert.True(update.Parameters[0].IsByReference);
        Assert.True(update.Parameters[1].IsIn);
        Assert.Equal(1, echo.GenericArity);

        var wrongPassingMode = Assert.Throws<BindingDiagnosticException>(() => reader.ResolveMethod(
            service,
            "UpdateMoney",
            ["BasicAssembly.Domain.Money", "System.Decimal"]));
        var wrongGenericArity = Assert.Throws<BindingDiagnosticException>(() => reader.ResolveMethod(
            service,
            "Echo",
            ["T"],
            genericArity: 0));

        Assert.Equal("NWB2014", wrongPassingMode.Diagnostic.Id);
        Assert.Equal("NWB2014", wrongGenericArity.Diagnostic.Id);
    }
}

internal static class AssemblyReaderTestExtensions
{
    private static readonly SourceLocation s_location = new("AssemblyInspectionTests.cs", 1, 1);

    public static InspectedType ResolveType(
        this AssemblyReader reader,
        InspectedAssembly assembly,
        string fullName) =>
        reader.ResolveType(assembly, fullName, s_location);

    public static InspectedMethod ResolveMethod(
        this AssemblyReader reader,
        InspectedType type,
        string name,
        IReadOnlyList<string> parameterTypeNames,
        bool? isStatic = null,
        IReadOnlyList<ParameterPassingMode>? parameterPassingModes = null,
        int? genericArity = null) =>
        reader.ResolveMethod(type, name, parameterTypeNames, s_location, isStatic, parameterPassingModes, genericArity);

    public static InspectedMethod ResolveMethodByName(
        this AssemblyReader reader,
        InspectedType type,
        string name,
        bool? isStatic = null) =>
        reader.ResolveMethodByName(type, name, s_location, isStatic);
}
