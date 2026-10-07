using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class ManagedBindingEmitter
{
    public static string Emit(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string fingerprint)
    {
        var simpleTypeName = type.Name.Split('.').Last();

        return $$"""
using System;

namespace {{model.Name}};

public static class {{simpleTypeName}}Bindings
{
    public const string ContractFingerprint = "{{fingerprint}}";

    public static decimal Calculate()
    {
        throw new NotSupportedException("This generated binding is only a compilation fixture.");
    }
}
""";
    }

    public static string EmitProject(BindingModel model, AssemblyIdentity identity)
    {
        return $$"""
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
    <AssemblyName>{{model.Name}}.Bindings</AssemblyName>
    <RootNamespace>{{model.Name}}</RootNamespace>
  </PropertyGroup>
  <ItemGroup>
    <Reference Include="BasicAssembly">
      <HintPath>{{identity.Location}}</HintPath>
    </Reference>
  </ItemGroup>
</Project>
""";
    }
}
