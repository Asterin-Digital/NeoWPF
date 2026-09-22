using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed class BindingValidation
{
    public IReadOnlyList<BindingDiagnostic> Validate(BindingModel model)
    {
        var diagnostics = new List<BindingDiagnostic>();
        ValidateTypes(model, diagnostics);
        return diagnostics;
    }

    private static void ValidateTypes(BindingModel model, ICollection<BindingDiagnostic> diagnostics)
    {
        AddDuplicateDiagnostics(model.Types, type => type.ManagedName, "NWB2001", "type", diagnostics);

        foreach (var type in model.Types)
        {
            AddDuplicateDiagnostics(type.Methods, method => method.NativeName, "NWB2002", "method", diagnostics);
            AddDuplicateDiagnostics(type.Properties, property => property.NativeName, "NWB2003", "property", diagnostics);
            AddDuplicateDiagnostics(type.Events, @event => @event.NativeName, "NWB2004", "event", diagnostics);

            foreach (var method in type.Methods)
                ValidateMethod(method, diagnostics);

            foreach (var @event in type.Events)
                ValidateEvent(@event, diagnostics);
        }
    }

    private static void ValidateMethod(ManagedMethodModel method, ICollection<BindingDiagnostic> diagnostics)
    {
        if (method.IsAsync && !IsTaskType(method.ReturnType))
        {
            diagnostics.Add(new BindingDiagnostic(
                "NWB4025",
                BindingDiagnosticSeverity.Error,
                "An async method must return Task<T> or ValueTask<T>.",
                method.Location));
        }

        if (!method.IsAsync && IsTaskType(method.ReturnType))
        {
            diagnostics.Add(new BindingDiagnostic(
                "NWB4026",
                BindingDiagnosticSeverity.Error,
                "Task and ValueTask returns must use 'returns async'.",
                method.Location));
        }

        foreach (var parameter in method.Parameters)
        {
            if (parameter.PassingMode is ParameterPassingMode.Ref or ParameterPassingMode.Out)
            {
                diagnostics.Add(new BindingDiagnostic(
                    "NWB4020",
                    BindingDiagnosticSeverity.Error,
                    "By-reference parameters are not generated yet.",
                    parameter.Location));
            }
        }
    }

    private static void ValidateEvent(ManagedEventModel @event, ICollection<BindingDiagnostic> diagnostics)
    {
        foreach (var parameter in @event.Parameters)
        {
            if (parameter.PassingMode is not ParameterPassingMode.Value)
            {
                diagnostics.Add(new BindingDiagnostic(
                    "NWB4027",
                    BindingDiagnosticSeverity.Error,
                    "Event parameters must be passed by value.",
                    parameter.Location));
            }
        }
    }

    private static bool IsTaskType(CanonicalType? type) =>
        type is { Kind: CanonicalTypeKind.Task or CanonicalTypeKind.ValueTask };

    private static void AddDuplicateDiagnostics<T>(
        IEnumerable<T> values,
        Func<T, string> keySelector,
        string diagnosticId,
        string description,
        ICollection<BindingDiagnostic> diagnostics)
    {
        foreach (var group in values.GroupBy(keySelector, StringComparer.Ordinal).Where(group => group.Count() > 1))
        {
            foreach (var duplicate in group.Skip(1))
            {
                var location = duplicate switch
                {
                    ManagedTypeModel type => type.Location,
                    ManagedMethodModel method => method.Location,
                    ManagedPropertyModel property => property.Location,
                    ManagedEventModel @event => @event.Location,
                    _ => new SourceLocation(string.Empty, 1, 1),
                };
                diagnostics.Add(new BindingDiagnostic(
                    diagnosticId,
                    BindingDiagnosticSeverity.Error,
                    $"Duplicate {description} '{group.Key}'.",
                    location));
            }
        }
    }
}
