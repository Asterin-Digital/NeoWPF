using System.Security.Cryptography;
using System.Text;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Emission;
using NeoWPF.DotNet.Interop.Generator.Parsing;

namespace NeoWPF.DotNet.Interop.Generator.Cli;

public sealed record GeneratorFile(string Path, string Content);

public sealed record GeneratorOutput(
    IReadOnlyList<GeneratorFile> NativeFiles,
    IReadOnlyList<GeneratorFile> ManagedFiles,
    string ContractText,
    string Fingerprint);

public static class GeneratorCommand
{
    public static GeneratorOutput Generate(GeneratorOptions options)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(options.BindingSource);
        ArgumentException.ThrowIfNullOrWhiteSpace(options.AssemblyPath);
        ArgumentException.ThrowIfNullOrWhiteSpace(options.OutputDirectory);

        var bindingPath = Path.Combine(options.OutputDirectory, "binding.nwbind");
        var syntax = new BindingParser(options.BindingSource, bindingPath).ParseDocument();
        var model = new BindingModelBuilder().Build(syntax.Bindings.Single());
        var diagnostics = new BindingValidation().Validate(model);
        if (diagnostics.Count != 0)
            throw new BindingParseException(diagnostics[0]);

        var assemblyReader = new AssemblyReader();
        var assembly = assemblyReader.Read(options.AssemblyPath);
        var bindingType = model.Types.Single();
        var type = new CanonicalTypeResolver().Resolve(
            CanonicalType.Named(CanonicalTypeKind.NamedClass, bindingType.ManagedName, [], null, bindingType.Location),
            assembly);
        var inspectedType = assembly.Types.Single(candidate => candidate.FullName == type.Name);
        var bindingMethod = bindingType.Methods.Single();
        var parameterTypes = bindingMethod.Parameters
            .Select(parameter => GetReflectionTypeName(parameter.Type))
            .ToArray();
        var method = bindingMethod.HasExplicitSignature
            ? assemblyReader.ResolveMethod(
                inspectedType,
                bindingMethod.ManagedName,
                parameterTypes,
                isStatic: null,
                parameterPassingModes: bindingMethod.Parameters.Select(parameter => parameter.PassingMode).ToArray())
            : assemblyReader.ResolveMethodByName(inspectedType, bindingMethod.ManagedName);

        if (!method.IsStatic || method.ReturnTypeName != "System.Int32" ||
            method.Parameters.Count != 1 || method.Parameters[0].TypeName != "System.Int32" ||
            method.Parameters[0].IsByReference)
        {
            throw new NotSupportedException(
                "This generator slice supports only static methods with one int32 parameter and an int32 return value.");
        }

        var contract = BuildContract(model, bindingType, bindingMethod, type, method, assembly.Identity);
        var fingerprint = ComputeFingerprint(contract);
        var nativeFiles = new List<GeneratorFile>();
        var managedFiles = new List<GeneratorFile>();

        Directory.CreateDirectory(options.OutputDirectory);
        if (options.GenerateNative)
        {
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{bindingType.NativeName}.g.h"),
                NativeBindingEmitter.Emit(model, type, method, bindingMethod.NativeName, fingerprint)));
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{bindingType.NativeName}.g.cpp"),
                NativeBindingEmitter.EmitImplementation(
                    model,
                    type,
                    method,
                    bindingMethod.NativeName,
                    fingerprint,
                    assembly.Identity)));
        }

        if (options.GenerateManaged)
        {
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{bindingType.NativeName}.Bindings.g.cs"),
                ManagedBindingEmitter.Emit(model, fingerprint)));
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{model.Name}.Bindings.csproj"),
                ManagedBindingEmitter.EmitProject(model, assembly.Identity)));
        }

        foreach (var file in nativeFiles.Concat(managedFiles))
            File.WriteAllText(file.Path, file.Content, Encoding.UTF8);

        var contractPath = Path.Combine(options.OutputDirectory, options.ContractFileName);
        File.WriteAllText(contractPath, contract, Encoding.UTF8);

        return new GeneratorOutput(nativeFiles, managedFiles, contract, fingerprint);
    }

    private static string BuildContract(
        BindingModel model,
        ManagedTypeModel bindingType,
        ManagedMethodModel bindingMethod,
        CanonicalType type,
        InspectedMethod method,
        AssemblyIdentity identity)
    {
        var builder = new StringBuilder();
        builder.AppendLine($"binding={model.Name}");
        builder.AppendLine($"assembly={Path.GetFullPath(identity.Location)}");
        builder.AppendLine($"assembly-name={identity.Name}");
        builder.AppendLine($"assembly-mvid={identity.ModuleVersionId}");
        builder.AppendLine($"binding-version={model.Version ?? string.Empty}");
        builder.AppendLine($"type={type.Name}");
        builder.AppendLine($"native-type={bindingType.NativeName}");
        builder.AppendLine($"method={method.Name}");
        builder.AppendLine($"method-static={method.IsStatic.ToString().ToLowerInvariant()}");
        builder.AppendLine($"native-method={bindingMethod.NativeName}");
        builder.AppendLine($"return={method.ReturnTypeName}");
        builder.AppendLine($"execution={bindingMethod.Execution}");
        foreach (var parameter in method.Parameters)
            builder.AppendLine($"parameter={parameter.TypeName}:{parameter.IsByReference}:{parameter.IsOut}:{parameter.IsIn}");
        builder.AppendLine("contract-version=1");
        builder.AppendLine("generator-version=NeoWPF.DotNet.Generator");
        return builder.ToString();
    }

    private static string ComputeFingerprint(string contract)
    {
        var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(contract));
        return Convert.ToHexString(bytes);
    }

    private static string GetReflectionTypeName(CanonicalType type) => type.Kind switch
    {
        CanonicalTypeKind.Void => "System.Void",
        CanonicalTypeKind.Boolean => "System.Boolean",
        CanonicalTypeKind.SignedInteger => type.IntegerWidth switch
        {
            IntegerWidth.Int8 => "System.SByte",
            IntegerWidth.Int16 => "System.Int16",
            IntegerWidth.Int32 => "System.Int32",
            IntegerWidth.Int64 => "System.Int64",
            _ => throw new NotSupportedException($"Integer width '{type.IntegerWidth}' is not supported."),
        },
        CanonicalTypeKind.UnsignedInteger => type.IntegerWidth switch
        {
            IntegerWidth.UInt8 => "System.Byte",
            IntegerWidth.UInt16 => "System.UInt16",
            IntegerWidth.UInt32 => "System.UInt32",
            IntegerWidth.UInt64 => "System.UInt64",
            _ => throw new NotSupportedException($"Integer width '{type.IntegerWidth}' is not supported."),
        },
        CanonicalTypeKind.FloatingPoint => type.Name switch
        {
            "float" => "System.Single",
            "double" => "System.Double",
            _ => throw new NotSupportedException($"Floating-point type '{type.Name}' is not supported."),
        },
        CanonicalTypeKind.Decimal => "System.Decimal",
        CanonicalTypeKind.String => "System.String",
        CanonicalTypeKind.NamedClass or
        CanonicalTypeKind.NamedStruct or
        CanonicalTypeKind.Enum or
        CanonicalTypeKind.Delegate or
        CanonicalTypeKind.Generic => type.Name,
        _ => throw new NotSupportedException($"Parameter type '{type.Kind}' is not supported by this generator slice."),
    };
}
