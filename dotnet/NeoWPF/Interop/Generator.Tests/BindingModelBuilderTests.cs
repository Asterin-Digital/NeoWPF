using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
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
                    method Calculate(Acme.Request request) returns owned System.Threading.Tasks.Task<int32>
                    {
                        execution Background;
                        managedName "ManagedCalculate";
                        nativeName "NativeCalculate";
                        adapter "ServiceAdapter.cs";
                    }
                    property Total
                    {
                        type decimal;
                        readable false;
                        writable true;
                        managedName "ManagedTotal";
                        nativeName "NativeTotal";
                    }
                    event Changed
                    {
                        callback Background;
                        managedName "ManagedChanged";
                        nativeName "NativeChanged";
                    }
                }
            }
            """, "Acme.nwbind").ParseDocument().Bindings[0];

        var model = new BindingModelBuilder().Build(syntax);
        var type = Assert.Single(model.Types);
        var method = Assert.Single(type.Methods);
        var property = Assert.Single(type.Properties);
        var @event = Assert.Single(type.Events);

        Assert.Equal("Acme.dll", model.AssemblyPath);
        Assert.Equal("ManagedCalculate", method.ManagedName);
        Assert.Equal("NativeCalculate", method.NativeName);
        Assert.Equal(ManagedExecutionPolicy.Background, method.Execution);
        Assert.Equal(ManagedOwnership.Owned, method.ReturnOwnership);
        Assert.Equal(CanonicalTypeKind.Task, method.ReturnType!.Kind);
        Assert.Equal("ManagedTotal", property.ManagedName);
        Assert.Equal("NativeTotal", property.NativeName);
        Assert.Equal(CanonicalTypeKind.Decimal, property.Type!.Kind);
        Assert.False(property.Readable);
        Assert.True(property.Writable);
        Assert.Equal("ManagedChanged", @event.ManagedName);
        Assert.Equal("NativeChanged", @event.NativeName);
        Assert.Equal(ManagedCallbackPolicy.Background, @event.Callback);
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

        var exception = Assert.Throws<BindingDiagnosticException>(() => new BindingModelBuilder().Build(syntax));

        Assert.Equal("NWB3010", exception.Diagnostic.Id);
    }
}
