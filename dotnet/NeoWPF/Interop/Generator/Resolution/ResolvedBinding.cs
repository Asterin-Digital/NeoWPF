using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Resolution;

public sealed record ResolvedBinding(
    BindingModel Binding,
    AssemblyIdentity PrimaryAssembly,
    IReadOnlyList<AssemblyIdentity> Dependencies,
    IReadOnlyList<ResolvedType> Types);

public sealed record ResolvedType(
    ManagedTypeModel Declaration,
    CanonicalType Type,
    InspectedType ManagedType,
    IReadOnlyList<ResolvedMethod> Methods);

public sealed record ResolvedMethod(
    ManagedTypeModel TypeDeclaration,
    CanonicalType Type,
    ManagedMethodModel Declaration,
    InspectedMethod Method,
    IReadOnlyList<CanonicalType> ParameterTypes,
    CanonicalType ReturnType);
