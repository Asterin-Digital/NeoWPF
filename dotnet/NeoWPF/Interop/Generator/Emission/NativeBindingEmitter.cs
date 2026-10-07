using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class NativeBindingEmitter
{
    public static string Emit(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string fingerprint)
    {
        var simpleTypeName = type.Name.Split('.').Last();

        return $$"""
#pragma once

namespace {{model.Name}}
{
class {{simpleTypeName}}Binding
{
public:
    static constexpr const char* ContractFingerprint = "{{fingerprint}}";
    static NeoWPF::DotNet::ManagedResult<{{method.ReturnTypeName}}> Calculate();
};
}
""";
    }

    public static string EmitImplementation(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string fingerprint)
    {
        var simpleTypeName = type.Name.Split('.').Last();

        return $$"""
#include "{{model.Name}}.g.h"

namespace {{model.Name}}
{
NeoWPF::DotNet::ManagedResult<{{method.ReturnTypeName}}> {{simpleTypeName}}Binding::Calculate()
{
    return NeoWPF::DotNet::ManagedResult<{{method.ReturnTypeName}}>::Error("Not implemented");
}
}
""";
    }
}
