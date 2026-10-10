namespace NeoWPF.DotNet.Interop.Generator.Diagnostics;

public sealed class BindingDiagnosticException : Exception
{
    public BindingDiagnosticException(BindingDiagnostic diagnostic)
        : this([diagnostic])
    {
    }

    public BindingDiagnosticException(IReadOnlyList<BindingDiagnostic> diagnostics)
        : base(string.Join(Environment.NewLine, diagnostics.Select(Format)))
    {
        if (diagnostics.Count == 0)
            throw new ArgumentException("At least one diagnostic is required.", nameof(diagnostics));

        Diagnostics = diagnostics;
    }

    public IReadOnlyList<BindingDiagnostic> Diagnostics { get; }

    public BindingDiagnostic Diagnostic => Diagnostics[0];

    private static string Format(BindingDiagnostic diagnostic) =>
        $"{diagnostic.Id} at {diagnostic.Location.File}:{diagnostic.Location.Line}:{diagnostic.Location.Column}: {diagnostic.Message}";
}
