namespace NeoWPF.DotNet.Interop.Generator.Cli;

public sealed record GeneratorOptions(
    string BindingPath,
    string? AssemblyPath,
    string OutputDirectory,
    bool GenerateManaged = true,
    bool GenerateNative = true,
    string ContractFileName = "binding.contract");
