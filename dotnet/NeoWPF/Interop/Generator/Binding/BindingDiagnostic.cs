using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public enum BindingDiagnosticSeverity
{
    Error,
    Warning,
}

public sealed record BindingDiagnostic(
    string Id,
    BindingDiagnosticSeverity Severity,
    string Message,
    SourceLocation Location);

public sealed class BindingParseException : Exception
{
    public BindingParseException(BindingDiagnostic diagnostic)
        : base($"{diagnostic.Id} at {diagnostic.Location.File}:{diagnostic.Location.Line}:{diagnostic.Location.Column}: {diagnostic.Message}")
    {
        Diagnostic = diagnostic;
    }

    public BindingDiagnostic Diagnostic { get; }
}
