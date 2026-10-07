using System.Diagnostics;
using System.Reflection;
using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.Cli;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class BindingGenerationTests
{
    [Fact]
    public void GeneratesDeterministicNativeAndManagedBindings()
    {
        var source = """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    nativeName "InvoiceService";
                    method Calculate(BasicAssembly.Domain.InvoiceRequest request)
                    {
                        returns decimal;
                        execution Managed;
                    }
                }
            }
            """;
        var assemblyPath = typeof(InvoiceService).Assembly.Location;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");

        try
        {
            var options = new GeneratorOptions(
                source,
                assemblyPath,
                workspace,
                GenerateManaged: true,
                GenerateNative: true,
                ContractFileName: "BasicAssembly.contract");

            var first = GeneratorCommand.Generate(options);
            var second = GeneratorCommand.Generate(options);

            Assert.Equal(2, first.NativeFiles.Count);
            Assert.Equal(2, first.ManagedFiles.Count);
            Assert.True(File.Exists(Path.Combine(workspace, "BasicAssembly.contract")));
            Assert.Equal(first.NativeFiles.Select(file => file.Content), second.NativeFiles.Select(file => file.Content));
            Assert.Equal(first.ManagedFiles.Select(file => file.Content), second.ManagedFiles.Select(file => file.Content));
            Assert.Contains(first.NativeFiles, file => file.Path.EndsWith("InvoiceService.g.h", StringComparison.Ordinal));
            Assert.Contains(first.ManagedFiles, file => file.Path.EndsWith("InvoiceService.Bindings.g.cs", StringComparison.Ordinal));

            var managedProject = first.ManagedFiles
                .Single(file => file.Path.EndsWith("InvoiceService.Bindings.csproj", StringComparison.Ordinal))
                .Path;
            var build = Process.Start(new ProcessStartInfo
            {
                FileName = "dotnet",
                Arguments = $"build \"{managedProject}\" --nologo --configuration Debug",
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false
            });
            Assert.NotNull(build);
            using var output = build;
            var outputText = output.StandardOutput.ReadToEnd();
            var errorText = output.StandardError.ReadToEnd();
            output.WaitForExit();
            Assert.True(output.ExitCode == 0, $"Generated managed project build failed: {outputText}{errorText}");
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }
}
