using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

internal sealed class BindingSliceLimits
{
    public IReadOnlyList<BindingDiagnostic> Validate(BindingDocument document, string bindingPath)
    {
        var diagnostics = new List<BindingDiagnostic>();
        if (document.Bindings.Count != 1)
        {
            diagnostics.Add(Create(
                BindingDiagnosticIds.BindingCountLimit,
                "The current generator requires exactly one binding per file.",
                document.Bindings.FirstOrDefault()?.Location ?? new SourceLocation(bindingPath, 1, 1)));
            return diagnostics;
        }

        var binding = document.Bindings[0];
        foreach (var item in binding.Items)
        {
            if (item is not (AssemblyDeclaration or DependencyDeclaration or NativeNameDeclaration or VersionDeclaration or TypeDeclaration))
            {
                diagnostics.Add(Create(
                    BindingDiagnosticIds.UnsupportedFeature,
                    $"The '{item.GetType().Name.Replace("Declaration", string.Empty, StringComparison.Ordinal)}' binding option is not generated yet.",
                    item.Location));
            }
        }

        var types = binding.Items.OfType<TypeDeclaration>().ToArray();
        if (types.Length == 0)
        {
            diagnostics.Add(Create(
                BindingDiagnosticIds.TypeCountLimit,
                "The current generator requires at least one bound type.",
                binding.Location));
        }

        foreach (var type in types)
        {
            foreach (var item in type.Items)
            {
                if (item is not (TypeNativeNameDeclaration or MethodDeclaration))
                {
                    diagnostics.Add(Create(
                        BindingDiagnosticIds.UnsupportedFeature,
                        $"The '{item.GetType().Name.Replace("Declaration", string.Empty, StringComparison.Ordinal)}' type feature is not generated yet.",
                        item.Location));
                }
            }

            var methods = type.Items.OfType<MethodDeclaration>().ToArray();
            if (methods.Length != 1)
            {
                diagnostics.Add(Create(
                    BindingDiagnosticIds.MethodCountLimit,
                    $"The current generator requires exactly one method per bound type; '{type.Name}' declares {methods.Length}.",
                    methods.FirstOrDefault()?.Location ?? type.Location));
            }

            foreach (var method in methods)
            {
                if (method.IsAsync || method.Items.OfType<ReturnDeclaration>().Any(item => item.IsAsync))
                {
                    diagnostics.Add(Create(
                        BindingDiagnosticIds.UnsupportedFeature,
                        "Async methods are not generated yet.",
                        method.Location));
                }

                if (method.ReturnOwnership is not null ||
                    method.Items.OfType<ReturnDeclaration>().Any(item => item.Ownership is not null))
                {
                    diagnostics.Add(Create(
                        BindingDiagnosticIds.UnsupportedFeature,
                        "Return ownership is not generated yet.",
                        method.Location));
                }

                foreach (var execution in method.Items.OfType<ExecutionDeclaration>())
                {
                    if (execution.Policy is ManagedExecutionPolicy.Calling or
                        ManagedExecutionPolicy.Application or ManagedExecutionPolicy.Background)
                    {
                        diagnostics.Add(Create(
                            BindingDiagnosticIds.UnsupportedExecutionPolicy,
                            $"The '{execution.Policy}' execution policy is not supported by the current generated-call slice.",
                            execution.Location));
                    }
                }

                foreach (var item in method.Items)
                {
                    if (item is MemberAdapterDeclaration)
                    {
                        diagnostics.Add(Create(
                            BindingDiagnosticIds.UnsupportedFeature,
                            "Method adapters are not generated yet.",
                            item.Location));
                    }
                }
            }
        }

        return diagnostics;
    }

    private static BindingDiagnostic Create(string id, string message, SourceLocation location) =>
        new(id, BindingDiagnosticSeverity.Error, message, location);
}
