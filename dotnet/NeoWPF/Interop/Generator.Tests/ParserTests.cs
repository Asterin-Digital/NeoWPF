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

    [Fact]
    public void ParsesEveryDeclarationAndTypeForm()
    {
        var document = new BindingParser("""
            // binding-level declarations
            binding Application
            {
                assembly "libs/Acme\u002eBusiness.dll";
                dependency "libs/Acme.Common.dll";
                adapter "bindings/ServiceAdapter.cs";
                version "1.0";
                nativeName "AcmeNative";

                type Acme.Business.InvoiceService
                {
                    nativeName "InvoiceService";
                    constructible;
                    constructor();
                    constructor(string accountName);
                    constructor(ref int32 count, in string prefix, out bool created)
                    {
                        execution Managed;
                    }

                    method Calculate(Acme.Business.InvoiceRequest request)
                    {
                        returns Acme.Business.InvoiceResult;
                        execution Calling;
                    }
                    method Reset;
                    method Send(Acme.Request request) returns Acme.Response;
                    method Unnamed(Acme.Request);
                    method WithPrimitives(
                        bool enabled,
                        int8 signedByte,
                        uint8 unsignedByte,
                        int16 signedShort,
                        uint16 unsignedShort,
                        int32 signedInteger,
                        uint32 unsignedInteger,
                        int64 signedLong,
                        uint64 unsignedLong,
                        float singleValue,
                        double doubleValue,
                        decimal decimalValue,
                        string text,
                        bytes payload) returns void;
                    method Open
                    {
                        returns owned Acme.Stream;
                        execution Background;
                        managedName "OpenManaged";
                        nativeName "OpenNative";
                        adapter "bindings/OpenAdapter.cs";
                    }
                    method GetTuple() returns (int32, string)[];
                    method GetGeneric() returns System.Collections.Generic.Dictionary<string, System.Collections.Generic.List<int32?>>?;
                    method GetHandle() returns Acme.Handle*;
                    method GetVoid() returns void;
                    method GetMatrix() returns int32[,];
                    method GetAsync() returns async System.Threading.Tasks.Task<int32>;
                    method ForApplication
                    {
                        execution Application;
                    }

                    property decimal Total readonly borrowed;
                    property bool IsBusy writeonly;
                    property DisplayName
                    {
                        type string;
                        readable false;
                        writable true;
                        ownership shared;
                        managedName "Name";
                        nativeName "DisplayNameNative";
                    }

                    event ProgressChanged
                    {
                        parameter progress: int32;
                        callback Application;
                        managedName "Progress";
                        nativeName "ProgressNative";
                        adapter "bindings/ProgressAdapter.cs";
                    }
                    event BackgroundProgress
                    {
                        callback Background;
                    }
                    event ManagedProgress
                    {
                        callback Managed;
                    }
                }
            }

            binding Calling
            {
                assembly "Calling.dll";
                type Acme.Managed
                {
                    method Background;
                }
            }
            """, "grammar.nwbind").ParseDocument();

        Assert.Equal(2, document.Bindings.Count);
        var first = document.Bindings[0];
        Assert.Equal("Application", first.Name);
        Assert.Equal("libs/Acme.Business.dll", Assert.Single(first.Items.OfType<AssemblyDeclaration>()).Path);
        var type = Assert.Single(first.Items.OfType<TypeDeclaration>());
        Assert.Equal("Acme.Business.InvoiceService", type.Name);
        Assert.Equal(3, type.Items.OfType<ConstructorDeclaration>().Count());
        Assert.Equal(13, type.Items.OfType<MethodDeclaration>().Count());
        Assert.Equal(3, type.Items.OfType<PropertyDeclaration>().Count());
        Assert.Equal(3, type.Items.OfType<EventDeclaration>().Count());
        Assert.Null(type.Items.OfType<MethodDeclaration>().Single(method => method.Name == "Unnamed").Parameters[0].Name);
        Assert.True(type.Items.OfType<MethodDeclaration>().Single(method => method.Name == "GetAsync").IsAsync);
        var secondType = Assert.Single(document.Bindings[1].Items.OfType<TypeDeclaration>());
        Assert.Equal("Background", secondType.Items.OfType<MethodDeclaration>().Single().Name);
    }

    [Fact]
    public void NormalizesEscapedIdentifiersAndAllowsContextualNames()
    {
        var document = new BindingParser("""
            binding @type
            {
                assembly "escaped.dll";
                type Acme.@event
                {
                    method @execution(int32 @out);
                    method Managed;
                }
            }
            """).ParseDocument();

        Assert.Equal("type", document.Bindings[0].Name);
        var type = Assert.Single(document.Bindings[0].Items.OfType<TypeDeclaration>());
        Assert.Equal("Acme.event", type.Name);
        Assert.Equal("execution", type.Items.OfType<MethodDeclaration>().First().Name);
        Assert.Equal("out", type.Items.OfType<MethodDeclaration>().First().Parameters[0].Name);
        Assert.Equal("Managed", type.Items.OfType<MethodDeclaration>().Last().Name);
    }

    [Fact]
    public void RejectsNonAsciiIdentifiers()
    {
        var exception = Assert.Throws<BindingParseException>(() =>
            new BindingParser("binding Caf\u00e9 { assembly \"Cafe.dll\"; }").ParseDocument());

        Assert.Equal("NWB1010", exception.Diagnostic.Id);
    }

    [Fact]
    public void DecodesStringEscapesAndSkipsBothCommentForms()
    {
        var document = new BindingParser("""
            // line comment
            binding Escapes
            {
                /* block comment */
                assembly "a\\b\"c\n\t\u002e.dll";
            }
            """).ParseDocument();

        Assert.Equal("a\\b\"c\n\t..dll", Assert.Single(document.Bindings[0].Items.OfType<AssemblyDeclaration>()).Path);
    }

    [Theory]
    [MemberData(nameof(MalformedGrammarBodies))]
    public void RejectsMalformedGrammarProductions(string body)
    {
        var source = $"binding Test {{ {body} }}";
        var exception = Assert.Throws<BindingParseException>(() => new BindingParser(source).ParseDocument());

        Assert.StartsWith("NWB", exception.Diagnostic.Id);
    }

    public static TheoryData<string> MalformedGrammarBodies => new()
    {
        "assembly;",
        "assembly \"input.dll\"",
        "dependency;",
        "dependency \"dependency.dll\"",
        "adapter;",
        "adapter \"Adapter.cs\"",
        "version;",
        "version \"1.0\"",
        "nativeName;",
        "type { }",
        "type Missing { nativeName \"Missing\" }",
        "type Missing { constructible }",
        "type Missing { constructor; }",
        "type Missing { constructor(string name }",
        "type Missing { method; }",
        "type Missing { method Calculate( }",
        "type Missing { method Calculate() returns; }",
        "type Missing { property decimal; }",
        "type Missing { property ; }",
        "type Missing { event; }",
        "type Missing { method Calculate { returns; } }",
        "type Missing { method Calculate { parameter request: ; } }",
        "type Missing { method Calculate { type; } }",
        "type Missing { method Calculate { execution; } }",
        "type Missing { method Calculate { execution Unknown; } }",
        "type Missing { method Calculate { ownership unknown; } }",
        "type Missing { event Changed { callback Unknown; } }",
        "type Missing { property Name { readable maybe; } }",
        "type Missing { property Name { writable maybe; } }",
        "type Missing { method Calculate { managedName; } }",
        "type Missing { method Calculate { nativeName; } }",
        "type Missing { method Calculate { adapter; } }",
        "type Missing { method Calculate returns (int32); }",
        "type Missing { method Calculate() returns async owned int32; }",
        "type Missing { method Calculate { returns async owned int32; } }",
        "type Missing { method Calculate returns System.Collections.Generic.List<>; }",
        "type Missing { method Calculate returns int32<Item>; }",
        "type Missing { method Calculate returns int32[; }",
        "type Missing { method Calculate(int value); }",
        "type Missing { method Calculate(long value); }",
        "type Missing { method Calculate(byte value); }",
        "type Missing { method Calculate(object value); }",
        "type Missing { method type; }"
    };

    [Theory]
    [MemberData(nameof(InvalidLexicalInputs))]
    public void ReportsDiagnosticsForInvalidLexicalForms(string source, string expectedDiagnostic)
    {
        var exception = Assert.Throws<BindingParseException>(() => new BindingParser(source).ParseDocument());

        Assert.Equal(expectedDiagnostic, exception.Diagnostic.Id);
    }

    public static TheoryData<string, string> InvalidLexicalInputs => new()
    {
        { "binding Test { assembly \"bad\\q\"; }", "NWB1022" },
        { "binding Test { assembly \"bad\\u12G4\"; }", "NWB1024" },
        { "binding Test { assembly \"bad; }", "NWB1023" },
        { "binding Test { assembly \"bad\nnext\"; }", "NWB1020" },
        { "binding Test { /* unterminated", "NWB1030" },
        { "binding @ { }", "NWB1011" }
    };
}
