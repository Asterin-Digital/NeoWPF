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

        var assembly = new AssemblyReader().Read(options.AssemblyPath);
        var type = new CanonicalTypeResolver().Resolve(
            CanonicalType.Named(CanonicalTypeKind.NamedClass, model.Types[0].ManagedName, [], null, model.Location),
            assembly);
        var inspectedType = assembly.Types.Single(candidate => candidate.FullName == type.Name);
        var method = inspectedType.Methods
            .Single(candidate => candidate.Name == "Calculate" &&
                !candidate.IsConstructor &&
                candidate.Parameters.Select(parameter => parameter.TypeName)
                    .SequenceEqual(["BasicAssembly.Domain.InvoiceRequest"], StringComparer.Ordinal));

        var contract = BuildContract(model, type, method, assembly.Identity);
        var fingerprint = ComputeFingerprint(contract);
        var nativeFiles = new List<GeneratorFile>();
        var managedFiles = new List<GeneratorFile>();

        Directory.CreateDirectory(options.OutputDirectory);
        if (options.GenerateNative)
        {
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, "InvoiceService.g.h"),
                NativeBindingEmitter.Emit(model, type, method, fingerprint)));
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, "InvoiceService.g.cpp"),
                NativeBindingEmitter.EmitImplementation(model, type, method, fingerprint)));
        }

        if (options.GenerateManaged)
        {
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, "InvoiceService.Bindings.g.cs"),
                ManagedBindingEmitter.Emit(model, type, method, fingerprint)));
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, "InvoiceService.Bindings.csproj"),
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
        CanonicalType type,
        InspectedMethod method,
        AssemblyIdentity identity)
    {
        var builder = new StringBuilder();
        builder.AppendLine($"binding={model.Name}");
        builder.AppendLine($"assembly={model.AssemblyPath}");
        builder.AppendLine($"assembly-name={identity.Name}");
        builder.AppendLine($"assembly-mvid={identity.ModuleVersionId}");
        builder.AppendLine($"type={type.Name}");
        builder.AppendLine($"method={method.Name}");
        builder.AppendLine($"return={method.ReturnTypeName}");
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
}
