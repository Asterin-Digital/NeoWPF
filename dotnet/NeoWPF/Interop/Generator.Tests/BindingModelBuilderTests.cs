using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class BindingModelBuilderTests
{
    [Fact]
    public void ConvertsSyntaxIntoCanonicalModel()
    {
        var syntax = new BindingParser("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    nativeName "Service";
                    method Calculate(Acme.Request request) returns async owned System.Threading.Tasks.Task<int32>
                    {
                        execution Background;
                        adapter "ServiceAdapter.cs";
                    }
                    property decimal Total readonly;
                }
            }
            """, "Acme.nwbind").ParseDocument().Bindings[0];

        var model = new BindingModelBuilder().Build(syntax);
        var type = Assert.Single(model.Types);
        var method = Assert.Single(type.Methods);
        var property = Assert.Single(type.Properties);

        Assert.Equal("Acme.dll", model.AssemblyPath);
        Assert.Equal(ManagedExecutionPolicy.Background, method.Execution);
        Assert.Equal(ManagedOwnership.Owned, method.ReturnOwnership);
        Assert.Equal(CanonicalTypeKind.Task, method.ReturnType!.Kind);
        Assert.Equal(CanonicalTypeKind.Decimal, property.Type!.Kind);
        Assert.False(property.Writable);
    }

    [Fact]
    public void RejectsPointerTypesWithStableDiagnostic()
    {
        var syntax = new BindingParser("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method Read(Acme.Pointer* value);
                }
            }
            """, "Acme.nwbind").ParseDocument().Bindings[0];

        var exception = Assert.Throws<BindingParseException>(() => new BindingModelBuilder().Build(syntax));

        Assert.Equal("NWB3010", exception.Diagnostic.Id);
    }
}
