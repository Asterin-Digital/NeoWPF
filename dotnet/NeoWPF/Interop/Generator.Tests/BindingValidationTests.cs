using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class BindingValidationTests
{
    [Fact]
    public void AcceptsAsyncResultTypeWithoutTaskWrapper()
    {
        var model = BuildModel("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method Calculate() returns async int32;
                }
            }
            """);

        var diagnostics = new BindingValidation().Validate(model);

        Assert.DoesNotContain(diagnostics, diagnostic => diagnostic.Id == BindingDiagnosticIds.AsyncReturnMismatch);
    }

    [Fact]
    public void RejectsTaskWrapperInAsyncResultDeclaration()
    {
        var model = BuildModel("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method Calculate() returns async System.Threading.Tasks.Task<int32>;
                }
            }
            """);

        var diagnostics = new BindingValidation().Validate(model);

        Assert.Contains(diagnostics, diagnostic => diagnostic.Id == BindingDiagnosticIds.AsyncReturnMismatch);
    }

    [Fact]
    public void RejectsTaskMethodsWithoutAsyncModifier()
    {
        var model = BuildModel("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method Calculate() returns System.Threading.Tasks.Task<int32>;
                }
            }
            """);

        var diagnostics = new BindingValidation().Validate(model);

        Assert.Contains(diagnostics, diagnostic => diagnostic.Id == "NWB4026");
    }

    [Fact]
    public void ReportsDuplicateProjectedMethods()
    {
        var model = BuildModel("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method First();
                    method Second()
                    {
                        nativeName "First";
                    }
                }
            }
            """);

        var diagnostics = new BindingValidation().Validate(model);

        Assert.Contains(diagnostics, diagnostic => diagnostic.Id == "NWB2002");
    }

    [Fact]
    public void ReportsByReferenceParametersAsNotYetGenerated()
    {
        var model = BuildModel("""
            binding Acme
            {
                assembly "Acme.dll";
                type Acme.Service
                {
                    method Update(ref int32 value);
                }
            }
            """);

        var diagnostics = new BindingValidation().Validate(model);

        Assert.Contains(diagnostics, diagnostic => diagnostic.Id == "NWB4020");
    }

    private static BindingModel BuildModel(string text)
    {
        var syntax = new BindingParser(text, "validation.nwbind").ParseDocument().Bindings[0];
        return new BindingModelBuilder().Build(syntax);
    }
}
