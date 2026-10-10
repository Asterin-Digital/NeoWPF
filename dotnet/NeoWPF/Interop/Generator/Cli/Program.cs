using NeoWPF.DotNet.Interop.Generator.Cli;

if (args.Length != 7 || args[0] != "generate")
{
    Console.Error.WriteLine(
        "Usage: NeoWPF.DotNet.Generator generate --binding <path> --assembly <path> --output <path>");
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
    !values.TryGetValue("--assembly", out var assemblyPath) ||
    !values.TryGetValue("--output", out var outputDirectory))
{
    Console.Error.WriteLine(
        "Usage: NeoWPF.DotNet.Generator generate --binding <path> --assembly <path> --output <path>");
    return 2;
}

GeneratorCommand.Generate(new GeneratorOptions(
    File.ReadAllText(bindingPath),
    assemblyPath,
    outputDirectory,
    GenerateManaged: true,
    GenerateNative: true,
    ContractFileName: "BasicAssembly.contract"));
return 0;