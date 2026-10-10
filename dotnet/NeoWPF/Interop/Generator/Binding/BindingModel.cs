using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Binding;

public sealed record BindingModel(
    string Name,
    string? Version,
    string? NativeName,
    string AssemblyPath,
    IReadOnlyList<string> Dependencies,
    IReadOnlyList<string> Adapters,
    IReadOnlyList<ManagedTypeModel> Types,
    SourceLocation Location);

public sealed record ManagedTypeModel(
    string ManagedName,
    string NativeName,
    bool IsConstructible,
    IReadOnlyList<ManagedConstructorModel> Constructors,
    IReadOnlyList<ManagedMethodModel> Methods,
    IReadOnlyList<ManagedPropertyModel> Properties,
    IReadOnlyList<ManagedEventModel> Events,
    SourceLocation Location);

public sealed record ManagedConstructorModel(
    IReadOnlyList<ManagedParameterModel> Parameters,
    ManagedExecutionPolicy Execution,
    SourceLocation Location);

public sealed record ManagedMethodModel(
    string ManagedName,
    string NativeName,
    IReadOnlyList<ManagedParameterModel> Parameters,
    bool HasExplicitSignature,
    CanonicalType? ReturnType,
    bool IsAsync,
    ManagedOwnership? ReturnOwnership,
    ManagedExecutionPolicy Execution,
    string? AdapterPath,
    SourceLocation Location);

public sealed record ManagedPropertyModel(
    string ManagedName,
    string NativeName,
    CanonicalType? Type,
    bool Readable,
    bool Writable,
    ManagedOwnership? Ownership,
    SourceLocation Location);

public sealed record ManagedEventModel(
    string ManagedName,
    string NativeName,
    ManagedCallbackPolicy Callback,
    IReadOnlyList<ManagedParameterModel> Parameters,
    SourceLocation Location);

public sealed record ManagedParameterModel(
    string? Name,
    CanonicalType Type,
    ParameterPassingMode PassingMode,
    SourceLocation Location);
