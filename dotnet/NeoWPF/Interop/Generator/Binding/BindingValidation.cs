using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed class BindingValidation
{
    public IReadOnlyList<BindingDiagnostic> Validate(BindingModel model)
    {
        var diagnostics = new List<BindingDiagnostic>();
        diagnostics.AddRange(new NativeNameValidation().Validate(model));
        ValidateTypes(model, diagnostics);
        return diagnostics;
    }

    private static void ValidateTypes(BindingModel model, ICollection<BindingDiagnostic> diagnostics)
    {
        AddDuplicateDiagnostics(model.Types, type => type.ManagedName, BindingDiagnosticIds.DuplicateType, "type", diagnostics);

        foreach (var type in model.Types)
        {
            AddDuplicateDiagnostics(
                type.Methods,
                method => $"{method.NativeName}({string.Join(",", method.Parameters.Select(parameter => TypeKey(parameter.Type)))})",
                BindingDiagnosticIds.DuplicateMethod,
                "method signature",
                diagnostics);
            AddDuplicateDiagnostics(type.Properties, property => property.NativeName, BindingDiagnosticIds.DuplicateProperty, "property", diagnostics);
            AddDuplicateDiagnostics(type.Events, @event => @event.NativeName, BindingDiagnosticIds.DuplicateEvent, "event", diagnostics);

            foreach (var method in type.Methods)
                ValidateMethod(method, diagnostics);

            foreach (var @event in type.Events)
                ValidateEvent(@event, diagnostics);
        }
    }

    private static void ValidateMethod(ManagedMethodModel method, ICollection<BindingDiagnostic> diagnostics)
    {
        if (method.IsAsync && IsTaskType(method.ReturnType))
        {
            diagnostics.Add(new BindingDiagnostic(
                BindingDiagnosticIds.AsyncReturnMismatch,
                BindingDiagnosticSeverity.Error,
            "An async binding return type is the task result type T, not Task<T> or ValueTask<T>.",
                method.Location));
        }

        if (!method.IsAsync && IsTaskType(method.ReturnType))
        {
            diagnostics.Add(new BindingDiagnostic(
                BindingDiagnosticIds.MissingAsyncMarker,
                BindingDiagnosticSeverity.Error,
                "Task and ValueTask returns must use 'returns async'.",
                method.Location));
        }

        foreach (var parameter in method.Parameters)
        {
            if (parameter.PassingMode is ParameterPassingMode.Ref or ParameterPassingMode.Out)
            {
                diagnostics.Add(new BindingDiagnostic(
                    BindingDiagnosticIds.UnsupportedByReference,
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
                    BindingDiagnosticIds.InvalidEventParameter,
                    BindingDiagnosticSeverity.Error,
                    "Event parameters must be passed by value.",
                    parameter.Location));
            }
        }
    }

    private static bool IsTaskType(CanonicalType? type) =>
        type is { Kind: CanonicalTypeKind.Task or CanonicalTypeKind.ValueTask };

    private static string TypeKey(CanonicalType type) =>
        $"{type.Kind}:{type.Name}:{type.IntegerWidth}:{type.ArrayRank}:{(type.Element is null ? string.Empty : TypeKey(type.Element))}:[{string.Join(",", type.Arguments.Select(TypeKey))}]";

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
