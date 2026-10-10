using NeoWPF.DotNet.Interop.Generator.Cli;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

if (args.Length < 5 || args[0] != "generate" || args.Length % 2 == 0)
{
    Console.Error.WriteLine(
        "Usage: NeoWPF.DotNet.Generator generate --binding <path> --output <path> [--assembly <path>]");
    return 2;
}

var values = new Dictionary<string, string>(StringComparer.Ordinal);
for (var index = 1; index < args.Length; index += 2)
{
    if (!args[index].StartsWith("--", StringComparison.Ordinal) || index + 1 >= args.Length ||
        !values.TryAdd(args[index], args[index + 1]))
    {
        Console.Error.WriteLine("Generator options must be unique name/value pairs.");
        return 2;
    }
}

if (!values.TryGetValue("--binding", out var bindingPath) ||
    !values.TryGetValue("--output", out var outputDirectory))
{
    Console.Error.WriteLine(
        "Usage: NeoWPF.DotNet.Generator generate --binding <path> --output <path> [--assembly <path>]");
    return 2;
}
values.TryGetValue("--assembly", out var assemblyPath);

try
{
    var options = new GeneratorOptions(
        bindingPath,
        assemblyPath,
        outputDirectory,
        GenerateManaged: true,
        GenerateNative: true,
        ContractFileName: "BasicAssembly.contract");
    var output = GeneratorCommand.Generate(options);
    GeneratorOutputWriter.Write(options, output);
    return 0;
}
catch (BindingDiagnosticException exception)
{
    foreach (var diagnostic in exception.Diagnostics)
    {
        Console.Error.WriteLine(
            $"{diagnostic.Id} at {diagnostic.Location.File}:{diagnostic.Location.Line}:{diagnostic.Location.Column}: {diagnostic.Message}");
    }
    return 1;
}