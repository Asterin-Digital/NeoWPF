using System.Diagnostics;
using System.Reflection;
using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.Cli;
using Xunit;

namespace NeoWPF.DotNet.Interop.Generator.Tests;

public sealed class BindingGenerationTests
{
    [Fact]
    public void IncludesStaticMethodShapeInContract()
    {
        var source = """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    nativeName "InvoiceService";
                    method AddTen(int32 value) returns int32;
                }
            }
            """;
        var assemblyPath = typeof(InvoiceService).Assembly.Location;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");

        try
        {
            var output = GeneratorCommand.Generate(new GeneratorOptions(
                source,
                assemblyPath,
                workspace,
                GenerateManaged: true,
                GenerateNative: true,
                ContractFileName: "BasicAssembly.contract"));

            Assert.Contains("method-static=true", output.ContractText);
            Assert.Contains("AddTen", output.NativeFiles.Single(file => file.Path.EndsWith(".g.cpp", StringComparison.Ordinal)).Content);
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    [Fact]
    public async Task GeneratesDeterministicNativeAndManagedBindings()
    {
        var source = """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    nativeName "RequestFactory";
                    method AddTen(int32 value) returns int32;
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
            var firstFiles = Directory.GetFiles(workspace)
                .ToDictionary(
                    path => Path.GetRelativePath(workspace, path),
                    path => Convert.ToHexString(File.ReadAllBytes(path)),
                    StringComparer.Ordinal);
            var second = GeneratorCommand.Generate(options);
            var secondFiles = Directory.GetFiles(workspace)
                .ToDictionary(
                    path => Path.GetRelativePath(workspace, path),
                    path => Convert.ToHexString(File.ReadAllBytes(path)),
                    StringComparer.Ordinal);

            Assert.Equal(2, first.NativeFiles.Count);
            Assert.Equal(2, first.ManagedFiles.Count);
            Assert.True(File.Exists(Path.Combine(workspace, "BasicAssembly.contract")));
            Assert.Contains("native-type=RequestFactory", first.ContractText);
            Assert.Contains("native-method=AddTen", first.ContractText);
            Assert.Contains("method-static=true", first.ContractText);
            Assert.Contains($"assembly={Path.GetFullPath(assemblyPath)}", first.ContractText);
            Assert.Equal(
                firstFiles.Keys.OrderBy(path => path, StringComparer.Ordinal),
                secondFiles.Keys.OrderBy(path => path, StringComparer.Ordinal));
            foreach (var path in firstFiles.Keys)
                Assert.Equal(firstFiles[path], secondFiles[path]);
            Assert.Equal(first.NativeFiles.Select(file => file.Content), second.NativeFiles.Select(file => file.Content));
            Assert.Equal(first.ManagedFiles.Select(file => file.Content), second.ManagedFiles.Select(file => file.Content));
            Assert.Contains(first.NativeFiles, file => file.Path.EndsWith("RequestFactory.g.h", StringComparison.Ordinal));
            Assert.Contains(first.ManagedFiles, file => file.Path.EndsWith("RequestFactory.Bindings.g.cs", StringComparison.Ordinal));
            var nativeHeader = first.NativeFiles
                .Single(file => file.Path.EndsWith("RequestFactory.g.h", StringComparison.Ordinal))
                .Content;
            Assert.Contains("NeoWPF::DotNet::ManagedResult<std::int32_t> AddTen(", nativeHeader);
            Assert.Contains("std::int32_t value) const;", nativeHeader);
            var nativeImplementation = first.NativeFiles
                .Single(file => file.Path.EndsWith("RequestFactory.g.cpp", StringComparison.Ordinal))
                .Content;
            Assert.Contains("#include \"RequestFactory.g.h\"", nativeImplementation);
            Assert.Contains("RequestFactoryBinding::AddTen(", nativeImplementation);
            Assert.Contains("std::int32_t value) const", nativeImplementation);
            Assert.Contains(
                "public const string ContractFingerprint",
                first.ManagedFiles.Single(file => file.Path.EndsWith("RequestFactory.Bindings.g.cs", StringComparison.Ordinal)).Content);

            var managedProject = first.ManagedFiles
                .Single(file => file.Path.EndsWith("BasicAssembly.Bindings.csproj", StringComparison.Ordinal))
                .Path;
            var managedProjectContent = File.ReadAllText(managedProject);
            Assert.Contains("<EnableDefaultCompileItems>false</EnableDefaultCompileItems>", managedProjectContent);
            Assert.DoesNotContain("<Compile Remove=", managedProjectContent);
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

            await CompileGeneratedNativeBinding(workspace);
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    private static async Task CompileGeneratedNativeBinding(string generatedDirectory)
    {
        var repositoryRoot = FindRepositoryRoot();
        var processInfo = new ProcessStartInfo
        {
            FileName = "python",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false
        };
        processInfo.ArgumentList.Add(Path.Combine(repositoryRoot, "private", "scripts", "Build.py"));
        processInfo.ArgumentList.Add("--source");
        processInfo.ArgumentList.Add(Path.Combine(repositoryRoot, "tests", "DotNetFixtures", "GeneratedNativeCompile"));
        processInfo.ArgumentList.Add("--build");
        processInfo.ArgumentList.Add(Path.Combine(generatedDirectory, "native-build"));
        processInfo.ArgumentList.Add("--config");
        processInfo.ArgumentList.Add("Debug");
        processInfo.ArgumentList.Add("--generator");
        processInfo.ArgumentList.Add("Ninja Multi-Config");
        processInfo.ArgumentList.Add("--target");
        processInfo.ArgumentList.Add("GeneratedNativeBinding");
        processInfo.ArgumentList.Add("-DNEOWPF_GENERATED_DIR=" + generatedDirectory);
        processInfo.ArgumentList.Add("-DNEOWPF_PUBLIC_INCLUDE_DIR=" + Path.Combine(repositoryRoot, "include"));

        using var process = Process.Start(processInfo);
        Assert.NotNull(process);
        var outputTask = process.StandardOutput.ReadToEndAsync();
        var errorTask = process.StandardError.ReadToEndAsync();
        await process.WaitForExitAsync();
        var outputText = await outputTask;
        var errorText = await errorTask;

        Assert.True(
            process.ExitCode == 0,
            $"Generated native binding compilation failed: {outputText}{errorText}");
    }

    private static string FindRepositoryRoot()
    {
        DirectoryInfo? directory = new(AppContext.BaseDirectory);
        while (directory is not null &&
            !File.Exists(Path.Combine(directory.FullName, "private", "scripts", "Build.py")))
        {
            directory = directory.Parent;
        }

        return directory?.FullName ?? throw new DirectoryNotFoundException(
            "Could not locate the NeoWPF repository root.");
    }
}
