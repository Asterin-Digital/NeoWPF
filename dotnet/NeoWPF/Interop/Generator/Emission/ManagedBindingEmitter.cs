using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class ManagedBindingEmitter
{
    public static string Emit(
        BindingModel model,
        string fingerprint)
    {
        var simpleTypeName = model.Types.Single().NativeName;

        return $$"""
namespace {{model.Name}};

public static class {{simpleTypeName}}Bindings
{
    public const string ContractFingerprint = "{{fingerprint}}";
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
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
    <AssemblyName>{{model.Name}}.Bindings</AssemblyName>
    <RootNamespace>{{model.Name}}</RootNamespace>
  </PropertyGroup>
  <ItemGroup>
    <Compile Include="{{model.Types.Single().NativeName}}.Bindings.g.cs" />
    <Reference Include="{{identity.Name}}">
      <HintPath>{{identity.Location}}</HintPath>
    </Reference>
  </ItemGroup>
</Project>
""";
    }
}
