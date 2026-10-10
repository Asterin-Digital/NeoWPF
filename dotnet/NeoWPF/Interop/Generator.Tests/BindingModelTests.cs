using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class BindingModelTests
{
    private static readonly SourceLocation Location = new("model.nwbind", 1, 1);

    [Fact]
    public void RepresentsRecursiveGenericArrayAndNullableTypes()
    {
        var stringType = CanonicalType.Primitive(CanonicalTypeKind.String, "string", IntegerWidth.None, Location);
        var listType = CanonicalType.Named(
            CanonicalTypeKind.Generic,
            "System.Collections.Generic.List",
            [stringType],
            "System.Runtime",
            Location);
        var nullableList = CanonicalType.Wrapped(CanonicalTypeKind.Nullable, listType, Location);
        var array = CanonicalType.CreateArray(nullableList, 1, Location);

        Assert.Equal(CanonicalTypeKind.Array, array.Kind);
        Assert.Equal(CanonicalTypeKind.Nullable, array.Element!.Kind);
        Assert.Equal(CanonicalTypeKind.Generic, array.Element.Element!.Kind);
        Assert.Equal("System.Collections.Generic.List", array.Element.Element.Name);
    }

    [Fact]
    public void KeepsExecutionAndOwnershipAsSeparatePolicies()
    {
        Assert.Equal(ManagedExecutionPolicy.Application, ManagedExecutionPolicy.Application);
        Assert.Equal(ManagedOwnership.Owned, ManagedOwnership.Owned);
        Assert.Equal(ManagedCallbackPolicy.Managed, ManagedCallbackPolicy.Managed);
    }
}
