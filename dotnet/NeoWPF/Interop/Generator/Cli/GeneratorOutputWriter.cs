using System.Text;

namespace NeoWPF.DotNet.Interop.Generator.Cli;

public static class GeneratorOutputWriter
{
    private static readonly Encoding s_utf8WithoutBom = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false);

    public static void Write(GeneratorOptions options, GeneratorOutput output)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(options.OutputDirectory);
        Directory.CreateDirectory(options.OutputDirectory);

        foreach (var file in output.NativeFiles.Concat(output.ManagedFiles))
            WriteIfChanged(file.Path, file.Content);

        WriteIfChanged(Path.Combine(options.OutputDirectory, options.ContractFileName), output.ContractText);
    }

    private static void WriteIfChanged(string path, string content)
    {
        var normalizedContent = content.Replace("\r\n", "\n", StringComparison.Ordinal)
            .Replace('\r', '\n');
        var bytes = s_utf8WithoutBom.GetBytes(normalizedContent);
        if (File.Exists(path) && File.ReadAllBytes(path).AsSpan().SequenceEqual(bytes))
            return;

        File.WriteAllBytes(path, bytes);
    }
}