using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Resolution;

internal sealed class BindingResolver
{
    public ResolvedBinding Resolve(BindingModel binding, InspectedAssembly assembly)
    {
        var diagnostics = new List<BindingDiagnostic>();
        var resolvedTypes = new List<ResolvedType>();
        var typeResolver = new CanonicalTypeResolver();
        var reader = new AssemblyReader();

        foreach (var typeDeclaration in binding.Types)
        {
            try
            {
                var resolvedType = typeResolver.Resolve(
                    CanonicalType.Named(
                        CanonicalTypeKind.NamedClass,
                        typeDeclaration.ManagedName,
                        [],
                        null,
                        typeDeclaration.Location),
                    assembly);
                var managedType = reader.ResolveType(assembly, resolvedType.Name, typeDeclaration.Location);
                var resolvedMethods = new List<ResolvedMethod>();

                foreach (var methodDeclaration in typeDeclaration.Methods)
                {
                    try
                    {
                        var parameterTypes = methodDeclaration.Parameters
                            .Select(parameter => typeResolver.Resolve(parameter.Type, assembly))
                            .ToArray();
                        var method = methodDeclaration.HasExplicitSignature
                            ? reader.ResolveMethodBySignature(
                                managedType,
                                methodDeclaration.ManagedName,
                                parameterTypes,
                                methodDeclaration.Location,
                                parameterPassingModes: methodDeclaration.Parameters
                                    .Select(parameter => parameter.PassingMode)
                                    .ToArray())
                            : reader.ResolveMethodByName(
                                managedType,
                                methodDeclaration.ManagedName,
                                methodDeclaration.Location);

                        if (methodDeclaration.ReturnType is null)
                        {
                            throw Unsupported(
                                "The current generated-call slice requires an explicitly declared return type.",
                                methodDeclaration.Location);
                        }

                        var returnType = typeResolver.Resolve(methodDeclaration.ReturnType, assembly);
                        var returnMatches = methodDeclaration.IsAsync
                            ? AssemblyReader.AsyncResultMatches(returnType, method.ReturnType)
                            : AssemblyReader.TypeMatches(returnType, method.ReturnType);
                        if (!returnMatches)
                        {
                            throw new BindingDiagnosticException(new BindingDiagnostic(
                                BindingDiagnosticIds.ReturnTypeMismatch,
                                BindingDiagnosticSeverity.Error,
                                $"Declared return type '{returnType.Name}' does not match managed return type '{method.ReturnTypeName}'.",
                                methodDeclaration.Location));
                        }

                        if (!method.IsStatic || method.ReturnTypeName != "System.Int32" ||
                            method.Parameters.Count != 1 || method.Parameters[0].TypeName != "System.Int32" ||
                            method.Parameters[0].IsByReference)
                        {
                            throw Unsupported(
                                "This generator slice supports one static int32-to-int32 method per bound type.",
                                methodDeclaration.Location);
                        }

                        resolvedMethods.Add(new ResolvedMethod(
                            typeDeclaration,
                            resolvedType,
                            methodDeclaration,
                            method,
                            parameterTypes,
                            returnType));
                    }
                    catch (BindingDiagnosticException exception)
                    {
                        diagnostics.AddRange(exception.Diagnostics);
                    }
                }

                resolvedTypes.Add(new ResolvedType(typeDeclaration, resolvedType, managedType, resolvedMethods));
            }
            catch (BindingDiagnosticException exception)
            {
                diagnostics.AddRange(exception.Diagnostics);
            }
        }

        if (diagnostics.Count != 0)
            throw new BindingDiagnosticException(diagnostics);

        return new ResolvedBinding(binding, assembly.Identity, assembly.Dependencies, resolvedTypes);
    }

    private static BindingDiagnosticException Unsupported(string message, SourceLocation location) =>
        new(new BindingDiagnostic(
            BindingDiagnosticIds.UnsupportedFeature,
            BindingDiagnosticSeverity.Error,
            message,
            location));
}
