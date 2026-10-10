using System.Reflection;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class DiagnosticIdTests
{
    [Fact]
    public void RegistryIdsAreUniqueAndComplete()
    {
        var definedIds = typeof(BindingDiagnosticIds)
            .GetFields(BindingFlags.Public | BindingFlags.Static)
            .Where(field => field.IsLiteral && !field.IsInitOnly)
            .Select(field => (string)field.GetRawConstantValue()!)
            .ToArray();

        Assert.Equal(definedIds.Length, definedIds.Distinct(StringComparer.Ordinal).Count());
        Assert.Equal(
            definedIds.OrderBy(id => id, StringComparer.Ordinal),
            BindingDiagnosticIds.All.OrderBy(id => id, StringComparer.Ordinal));
    }
}
