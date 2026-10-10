using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Parsing;

public abstract record BindingNode(SourceLocation Location);

public sealed record BindingDocument(IReadOnlyList<BindingDeclaration> Bindings)
    : BindingNode(Bindings.Count == 0 ? new SourceLocation(string.Empty, 1, 1) : Bindings[0].Location);

public sealed record BindingDeclaration(
    string Name,
    IReadOnlyList<BindingItem> Items,
    SourceLocation Location) : BindingNode(Location);

public abstract record BindingItem(SourceLocation Location) : BindingNode(Location);

public sealed record AssemblyDeclaration(string Path, SourceLocation Location) : BindingItem(Location);
public sealed record DependencyDeclaration(string Path, SourceLocation Location) : BindingItem(Location);
public sealed record AdapterDeclaration(string Path, SourceLocation Location) : BindingItem(Location);
public sealed record VersionDeclaration(string Value, SourceLocation Location) : BindingItem(Location);
public sealed record NativeNameDeclaration(string Name, SourceLocation Location) : BindingItem(Location);
public sealed record TypeDeclaration(
    string Name,
    IReadOnlyList<TypeItem> Items,
    SourceLocation Location) : BindingItem(Location);

public abstract record TypeItem(SourceLocation Location) : BindingNode(Location);
public sealed record TypeNativeNameDeclaration(string Name, SourceLocation Location) : TypeItem(Location);
public sealed record ConstructibleDeclaration(SourceLocation Location) : TypeItem(Location);
public sealed record ConstructorDeclaration(
    IReadOnlyList<ParameterSyntax> Parameters,
    IReadOnlyList<MemberItem> Items,
    SourceLocation Location) : TypeItem(Location);
public sealed record MethodDeclaration(
    string Name,
    IReadOnlyList<ParameterSyntax> Parameters,
    bool HasExplicitSignature,
    TypeSyntax? ReturnType,
    bool IsAsync,
    ManagedOwnership? ReturnOwnership,
    IReadOnlyList<MemberItem> Items,
    SourceLocation Location) : TypeItem(Location);
public sealed record PropertyDeclaration(
    string Name,
    TypeSyntax? Type,
    bool Readable,
    bool Writable,
    ManagedOwnership? Ownership,
    IReadOnlyList<MemberItem> Items,
    SourceLocation Location) : TypeItem(Location);
public sealed record EventDeclaration(
    string Name,
    IReadOnlyList<MemberItem> Items,
    SourceLocation Location) : TypeItem(Location);

public abstract record MemberItem(SourceLocation Location) : BindingNode(Location);
public sealed record ReturnDeclaration(TypeSyntax Type, bool IsAsync, ManagedOwnership? Ownership, SourceLocation Location) : MemberItem(Location);
public sealed record ParameterDeclaration(ParameterSyntax Parameter, SourceLocation Location) : MemberItem(Location);
public sealed record MemberTypeDeclaration(TypeSyntax Type, SourceLocation Location) : MemberItem(Location);
public sealed record ExecutionDeclaration(ManagedExecutionPolicy Policy, SourceLocation Location) : MemberItem(Location);
public sealed record OwnershipDeclaration(ManagedOwnership Ownership, SourceLocation Location) : MemberItem(Location);
public sealed record CallbackDeclaration(ManagedCallbackPolicy Policy, SourceLocation Location) : MemberItem(Location);
public sealed record ReadableDeclaration(bool Value, SourceLocation Location) : MemberItem(Location);
public sealed record WritableDeclaration(bool Value, SourceLocation Location) : MemberItem(Location);
public sealed record ManagedNameDeclaration(string Name, SourceLocation Location) : MemberItem(Location);
public sealed record MemberNativeNameDeclaration(string Name, SourceLocation Location) : MemberItem(Location);
public sealed record MemberAdapterDeclaration(string Path, SourceLocation Location) : MemberItem(Location);

public sealed record ParameterSyntax(ParameterPassingMode PassingMode, TypeSyntax Type, string? Name, SourceLocation Location);

public abstract record TypeSyntax(SourceLocation Location);
public sealed record NamedTypeSyntax(string Name, IReadOnlyList<TypeSyntax> Arguments, SourceLocation Location) : TypeSyntax(Location);
public sealed record PrimitiveTypeSyntax(string Name, SourceLocation Location) : TypeSyntax(Location);
public sealed record NullableTypeSyntax(TypeSyntax Element, SourceLocation Location) : TypeSyntax(Location);
public sealed record ArrayTypeSyntax(TypeSyntax Element, int Rank, SourceLocation Location) : TypeSyntax(Location);
public sealed record PointerTypeSyntax(TypeSyntax Element, SourceLocation Location) : TypeSyntax(Location);
public sealed record TupleTypeSyntax(IReadOnlyList<TypeSyntax> Elements, SourceLocation Location) : TypeSyntax(Location);

public enum ParameterPassingMode
{
    Value,
    Ref,
    In,
    Out,
}

public enum ManagedOwnership
{
    Owned,
    Borrowed,
    Shared,
}

public enum ManagedExecutionPolicy
{
    Unspecified,
    Managed,
    Calling,
    Application,
    Background,
}

public enum ManagedCallbackPolicy
{
    Application,
    Managed,
    Background,
}
