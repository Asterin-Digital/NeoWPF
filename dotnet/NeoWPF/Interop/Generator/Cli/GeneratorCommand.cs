using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Emission;
using NeoWPF.DotNet.Interop.Generator.Resolution;
using NeoWPF.DotNet.Interop.Generator.Parsing;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

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
        ArgumentException.ThrowIfNullOrWhiteSpace(options.BindingPath);
        ArgumentException.ThrowIfNullOrWhiteSpace(options.OutputDirectory);

        var bindingPath = Path.GetFullPath(options.BindingPath);
        var syntax = new BindingParser(File.ReadAllText(bindingPath), bindingPath).ParseDocument();
        var sliceDiagnostics = new BindingSliceLimits().Validate(syntax, bindingPath);
        if (sliceDiagnostics.Count != 0)
            throw new BindingDiagnosticException(sliceDiagnostics);

        var model = new BindingModelBuilder().Build(syntax.Bindings[0]);
        var diagnostics = new BindingValidation().Validate(model);
        if (diagnostics.Count != 0)
            throw new BindingDiagnosticException(diagnostics);

        var bindingDirectory = Path.GetDirectoryName(bindingPath)!;
        var assemblyPath = string.IsNullOrWhiteSpace(options.AssemblyPath)
            ? Path.GetFullPath(model.AssemblyPath, bindingDirectory)
            : Path.GetFullPath(options.AssemblyPath);
        var dependencyPaths = model.Dependencies
            .Select(path => Path.GetFullPath(path, bindingDirectory))
            .ToArray();
        var assemblyReader = new AssemblyReader();
        var assembly = assemblyReader.Read(assemblyPath, dependencyPaths);
        var resolvedBinding = new BindingResolver().Resolve(model, assembly);
        var (contract, fingerprint) = ContractFingerprint.Create(resolvedBinding);
        var contractAssemblies = ContractFingerprint.GetAssemblyIdentities(resolvedBinding);
        var nativeFiles = new List<GeneratorFile>();
        var managedFiles = new List<GeneratorFile>();

        if (options.GenerateNative)
        {
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{resolvedBinding.Binding.NativeName ?? model.Name}.g.h"),
                NativeBindingEmitter.Emit(resolvedBinding.Binding, resolvedBinding.Types, fingerprint)));
            nativeFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{resolvedBinding.Binding.NativeName ?? model.Name}.g.cpp"),
                NativeBindingSourceEmitter.Emit(
                    resolvedBinding.Binding,
                    resolvedBinding.Types,
                    fingerprint,
                    contractAssemblies)));
        }

        if (options.GenerateManaged)
        {
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{model.Name}.Contract.g.cs"),
                ManagedBindingEmitter.Emit(resolvedBinding.Binding, fingerprint, contractAssemblies)));
            managedFiles.Add(new GeneratorFile(
                Path.Combine(options.OutputDirectory, $"{model.Name}.Bindings.csproj"),
                BindingProjectEmitter.Emit(resolvedBinding.Binding, contractAssemblies)));
        }

        return new GeneratorOutput(nativeFiles, managedFiles, contract, fingerprint);
    }

}
