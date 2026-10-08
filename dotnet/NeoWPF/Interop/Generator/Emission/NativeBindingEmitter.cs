using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class NativeBindingEmitter
{
    private static string GetNativeTypeName(string managedTypeName) => managedTypeName switch
    {
        "System.Boolean" => "bool",
        "System.Byte" => "std::uint8_t",
        "System.SByte" => "std::int8_t",
        "System.Int16" => "std::int16_t",
        "System.UInt16" => "std::uint16_t",
        "System.Int32" => "std::int32_t",
        "System.UInt32" => "std::uint32_t",
        "System.Int64" => "std::int64_t",
        "System.UInt64" => "std::uint64_t",
        "System.Single" => "float",
        "System.Double" => "double",
        "System.Decimal" => "NeoWPF::DotNet::ManagedDecimal",
        "System.String" => "std::string",
        "System.Void" => "void",
        _ => "NeoWPF::DotNet::ManagedObjectReference"
    };

    private static string GetNativeParameter(InspectedParameter parameter)
    {
        var typeName = GetNativeTypeName(parameter.TypeName);
        var isReference = typeName is "NeoWPF::DotNet::ManagedDecimal" or "NeoWPF::DotNet::ManagedObjectReference";
        var parameterType = isReference ? $"const {typeName}&" : typeName;
        return $"{parameterType} {parameter.Name}";
    }

    public static string Emit(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string nativeMethodName,
        string fingerprint)
    {
        var simpleTypeName = model.Types.Single(candidate => candidate.ManagedName == type.Name).NativeName;
        var returnTypeName = GetNativeTypeName(method.ReturnTypeName);
        var parameters = string.Join(", ", method.Parameters.Select(GetNativeParameter));

        return $$"""
#pragma once

#include <NeoWPF/DotNet/Errors/ManagedResult.h>
#include <NeoWPF/DotNet/References/ManagedReference.h>
#include <NeoWPF/DotNet/Values/ManagedDecimal.h>

#include <cstdint>
#include <string>

namespace {{model.Name}}
{
class {{simpleTypeName}}Binding
{
public:
    static constexpr const char* ContractFingerprint = "{{fingerprint}}";
    static NeoWPF::DotNet::ManagedResult<{{returnTypeName}}> {{nativeMethodName}}({{parameters}});
};
}
""";
    }

    public static string EmitImplementation(
        BindingModel model,
        CanonicalType type,
        InspectedMethod method,
        string nativeMethodName,
        string fingerprint)
    {
        var simpleTypeName = model.Types.Single(candidate => candidate.ManagedName == type.Name).NativeName;
        var returnTypeName = GetNativeTypeName(method.ReturnTypeName);
        var parameters = string.Join(", ", method.Parameters.Select(GetNativeParameter));

        return $$"""
#include "{{simpleTypeName}}.g.h"

namespace {{model.Name}}
{
NeoWPF::DotNet::ManagedResult<{{returnTypeName}}> {{simpleTypeName}}Binding::{{nativeMethodName}}({{parameters}})
{
    return NeoWPF::DotNet::ManagedResult<{{returnTypeName}}>::Error("Not implemented");
}
}
""";
    }
}
