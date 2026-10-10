namespace NeoWPF.DotNet.Interop.Generator.Diagnostics;

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
