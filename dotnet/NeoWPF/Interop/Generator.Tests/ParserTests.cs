using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class ParserTests
{
    [Fact]
    public void ParsesTheDocumentExample()
    {
        var document = new BindingParser("""
binding AcmeBusiness
{
    assembly "libs/Acme.Business.dll";
    dependency "libs/Acme.Common.dll";

    type Acme.Business.InvoiceService
    {
        nativeName "InvoiceService";
        constructor(string accountName);
        method Calculate(Acme.Business.InvoiceRequest request)
        {
            returns Acme.Business.InvoiceResult;
            execution Managed;
        }
    }
}
""", "example.nwbind").ParseDocument();

        Assert.Single(document.Bindings);
        Assert.Equal("AcmeBusiness", document.Bindings[0].Name);
        var type = Assert.Single(document.Bindings[0].Items.OfType<TypeDeclaration>());
        Assert.Equal("Calculate", Assert.Single(type.Items.OfType<MethodDeclaration>()).Name);
    }

    [Fact]
    public void ReportsInvalidSyntax()
    {
        var exception = Assert.Throws<BindingParseException>(() =>
            new BindingParser("binding Broken { type Missing { method; } }").ParseDocument());

        Assert.Equal("NWB1010", exception.Diagnostic.Id);
    }
}
