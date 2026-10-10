using System.Diagnostics;
using System.Reflection;
using BasicAssembly.Domain;
using NeoWPF.DotNet.Interop.Generator.Cli;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;
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
        Directory.CreateDirectory(workspace);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, source);

        try
        {
            var output = GeneratorCommand.Generate(new GeneratorOptions(
            bindingPath,
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

    [Theory]
    [InlineData("method AddTen(int32 value) returns int32; event Changed;")]
    [InlineData("method AddTen(int32 value) returns int32; property int32 Count;")]
    [InlineData("method AddTen(int32 value) returns int32; constructor();")]
    [InlineData("method AddTen(int32 value) returns int32 { adapter \"ignored.cs\"; }")]
    public void RejectsDeclarationsNotGeneratedByTheCurrentSlice(string typeMembers)
    {
        var source = $$"""
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    {{typeMembers}}
                }
            }
            """;
        var assemblyPath = typeof(InvoiceService).Assembly.Location;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, source);

        try
        {
            var exception = Assert.Throws<BindingDiagnosticException>(() =>
                GeneratorCommand.Generate(new GeneratorOptions(bindingPath, assemblyPath, workspace)));

            Assert.Contains(exception.Diagnostics, diagnostic => diagnostic.Id == "NWB4034");
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    [Theory]
    [InlineData("string", "int32", "NWB3019")]
    [InlineData("int32", "string", "NWB3018")]
    public void RejectsDeclaredParameterAndReturnTypesThatDifferFromManagedSignature(
        string parameterType,
        string returnType,
        string expectedDiagnostic)
    {
        var source = $$"""
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    method AddTen({{parameterType}} value) returns {{returnType}};
                }
            }
            """;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, source);

        try
        {
            var exception = Assert.Throws<BindingDiagnosticException>(() =>
                GeneratorCommand.Generate(new GeneratorOptions(
                    bindingPath,
                    typeof(InvoiceService).Assembly.Location,
                    workspace)));

            Assert.Contains(exception.Diagnostics, diagnostic =>
                diagnostic.Id == expectedDiagnostic && diagnostic.Location.File == bindingPath);
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    [Fact]
    public void UsesAssemblyDeclaredRelativeToBindingWhenNoOverrideIsProvided()
    {
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var assemblyPath = Path.Combine(workspace, "BasicAssembly.dll");
        File.Copy(typeof(InvoiceService).Assembly.Location, assemblyPath);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    method AddTen(int32 value) returns int32;
                }
            }
            """);

        try
        {
            var output = GeneratorCommand.Generate(new GeneratorOptions(
                bindingPath,
                null,
                workspace,
                GenerateManaged: false,
                GenerateNative: false));

            Assert.Contains("assembly=BasicAssembly\n", output.ContractText);
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    [Fact]
    public void FingerprintDoesNotDependOnAssemblyPathOrMvid()
    {
        var source = """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                type BasicAssembly.Domain.InvoiceService
                {
                    method AddTen(int32 value) returns int32;
                }
            }
            """;
        var sourceAssembly = typeof(InvoiceService).Assembly.Location;
        var workspaces = new[]
        {
            Path.Combine(Path.GetTempPath(), $"neowpf-generator-a-{Guid.NewGuid():N}"),
            Path.Combine(Path.GetTempPath(), $"neowpf-generator-b-{Guid.NewGuid():N}"),
        };
        var outputs = new List<GeneratorOutput>();

        try
        {
            foreach (var workspace in workspaces)
            {
                Directory.CreateDirectory(workspace);
                File.Copy(sourceAssembly, Path.Combine(workspace, "BasicAssembly.dll"));
                var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
                File.WriteAllText(bindingPath, source);
                outputs.Add(GeneratorCommand.Generate(new GeneratorOptions(
                    bindingPath,
                    null,
                    workspace,
                    GenerateManaged: false,
                    GenerateNative: false)));
            }

            Assert.Equal(outputs[0].ContractText, outputs[1].ContractText);
            Assert.Equal(outputs[0].Fingerprint, outputs[1].Fingerprint);
            Assert.DoesNotContain("assembly-mvid=", outputs[0].ContractText);
            Assert.DoesNotContain(workspaces[0], outputs[0].ContractText, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain(workspaces[1], outputs[0].ContractText, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain('\r', outputs[0].ContractText);
            Assert.EndsWith("\n", outputs[0].ContractText);
        }
        finally
        {
            foreach (var workspace in workspaces)
            {
                if (Directory.Exists(workspace))
                    Directory.Delete(workspace, recursive: true);
            }
        }
    }

    [Fact]
    public void ResolvesDeclaredDependencyRelativeToBinding()
    {
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var pluginPath = Path.Combine(workspace, "PluginAssembly.dll");
        var dependencyPath = Path.Combine(workspace, "RenamedDependency.dll");
        File.Copy(typeof(PluginAssembly.PluginService).Assembly.Location, pluginPath);
        File.Copy(typeof(SecondaryDependency.Calculator).Assembly.Location, dependencyPath);
        var bindingPath = Path.Combine(workspace, "PluginAssembly.nwbind");
        File.WriteAllText(bindingPath, """
            binding PluginAssembly
            {
                assembly "PluginAssembly.dll";
                dependency "RenamedDependency.dll";
                type PluginAssembly.PluginService
                {
                    method AddFive(int32 value) returns int32;
                }
            }
            """);

        try
        {
            var output = GeneratorCommand.Generate(new GeneratorOptions(
                bindingPath,
                null,
                workspace,
                GenerateManaged: true,
                GenerateNative: true));

            Assert.Contains("assembly=PluginAssembly\n", output.ContractText);
            Assert.Contains("dependency=SecondaryDependency\n", output.ContractText);
            Assert.Contains("\"PluginAssembly\", \"SecondaryDependency\"", output.ManagedFiles
                .Single(file => file.Path.EndsWith(".Contract.g.cs", StringComparison.Ordinal)).Content);
            Assert.Contains("dependencyAssembly0", output.NativeFiles
                .Single(file => file.Path.EndsWith(".g.cpp", StringComparison.Ordinal)).Content);
            Assert.Contains("RenamedDependency.dll", output.NativeFiles
                .Single(file => file.Path.EndsWith(".g.cpp", StringComparison.Ordinal)).Content);
        }
        finally
        {
            if (Directory.Exists(workspace))
                Directory.Delete(workspace, recursive: true);
        }
    }

    [Fact]
    public void AccumulatesSliceDiagnosticsAtTheBindingPath()
    {
        var source = """
            binding BasicAssembly
            {
                assembly "BasicAssembly.dll";
                dependency "Secondary.dll";
                nativeName "AlternateNamespace";
                adapter "Adapter.cs";
                type BasicAssembly.Domain.InvoiceService
                {
                    method AddTen(int32 value) returns int32;
                }
            }
            """;
        var assemblyPath = typeof(InvoiceService).Assembly.Location;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, source);

        try
        {
            var exception = Assert.Throws<BindingDiagnosticException>(() =>
                GeneratorCommand.Generate(new GeneratorOptions(bindingPath, assemblyPath, workspace)));

            Assert.Single(exception.Diagnostics);
            Assert.All(exception.Diagnostics, diagnostic =>
            {
                Assert.Equal("NWB4034", diagnostic.Id);
                Assert.Equal(bindingPath, diagnostic.Location.File);
                Assert.True(diagnostic.Location.Line > 0);
                Assert.True(diagnostic.Location.Column > 0);
            });
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
                nativeName "RequestBindings";
                type BasicAssembly.Domain.InvoiceService
                {
                    nativeName "RequestFactory";
                    method AddTen(int32 value) returns int32;
                }
            }
            """;
        var assemblyPath = typeof(InvoiceService).Assembly.Location;
        var workspace = Path.Combine(Path.GetTempPath(), $"neowpf-generator-{Guid.NewGuid():N}");
        Directory.CreateDirectory(workspace);
        var bindingPath = Path.Combine(workspace, "BasicAssembly.nwbind");
        File.WriteAllText(bindingPath, source);

        try
        {
            var options = new GeneratorOptions(
            bindingPath,
                assemblyPath,
                workspace,
                GenerateManaged: true,
                GenerateNative: true,
                ContractFileName: "BasicAssembly.contract");

            var first = GeneratorCommand.Generate(options);
            GeneratorOutputWriter.Write(options, first);
            var firstFiles = Directory.GetFiles(workspace)
                .ToDictionary(
                    path => Path.GetRelativePath(workspace, path),
                    path => Convert.ToHexString(File.ReadAllBytes(path)),
                    StringComparer.Ordinal);
            var second = GeneratorCommand.Generate(options);
            var contractPath = Path.Combine(workspace, options.ContractFileName);
            var previousWriteTime = File.GetLastWriteTimeUtc(contractPath);
            GeneratorOutputWriter.Write(options, second);
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
            Assert.Contains("assembly=BasicAssembly\n", first.ContractText);
            Assert.DoesNotContain(Path.GetFullPath(assemblyPath), first.ContractText, StringComparison.OrdinalIgnoreCase);
            Assert.Equal(
                firstFiles.Keys.OrderBy(path => path, StringComparer.Ordinal),
                secondFiles.Keys.OrderBy(path => path, StringComparer.Ordinal));
            foreach (var path in firstFiles.Keys)
                Assert.Equal(firstFiles[path], secondFiles[path]);
            Assert.Equal(first.NativeFiles.Select(file => file.Content), second.NativeFiles.Select(file => file.Content));
            Assert.Equal(first.ManagedFiles.Select(file => file.Content), second.ManagedFiles.Select(file => file.Content));
            Assert.Equal(previousWriteTime, File.GetLastWriteTimeUtc(contractPath));
            Assert.False(File.ReadAllBytes(contractPath).AsSpan().StartsWith(new byte[] { 0xEF, 0xBB, 0xBF }));
            Assert.DoesNotContain('\r', File.ReadAllText(contractPath));
            Assert.Contains(first.NativeFiles, file => file.Path.EndsWith("RequestBindings.g.h", StringComparison.Ordinal));
            Assert.Contains(first.ManagedFiles, file => file.Path.EndsWith("BasicAssembly.Contract.g.cs", StringComparison.Ordinal));
            var nativeHeader = first.NativeFiles
                .Single(file => file.Path.EndsWith("RequestBindings.g.h", StringComparison.Ordinal))
                .Content;
            Assert.Contains("NeoWPF::DotNet::ManagedResult<std::int32_t> AddTen(", nativeHeader);
            Assert.Contains("std::int32_t template_) const;", nativeHeader);
            var nativeImplementation = first.NativeFiles
                .Single(file => file.Path.EndsWith("RequestBindings.g.cpp", StringComparison.Ordinal))
                .Content;
            Assert.Contains("#include <RequestBindings.g.h>", nativeImplementation);
            Assert.Contains("path.u8string()", nativeImplementation);
            Assert.DoesNotContain(".string()", nativeImplementation);
            Assert.Contains("RequestFactoryBinding::AddTen(", nativeImplementation);
            Assert.Contains("std::int32_t template_) const", nativeImplementation);
            Assert.Contains(
                "public static class BindingContract",
                first.ManagedFiles.Single(file => file.Path.EndsWith("BasicAssembly.Contract.g.cs", StringComparison.Ordinal)).Content);
            Assert.Contains("public static readonly string[] AssemblyNames", first.ManagedFiles.Single(file =>
                file.Path.EndsWith("BasicAssembly.Contract.g.cs", StringComparison.Ordinal)).Content);
            Assert.Contains("public static readonly string[] AssemblyMvids", first.ManagedFiles.Single(file =>
                file.Path.EndsWith("BasicAssembly.Contract.g.cs", StringComparison.Ordinal)).Content);

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
        processInfo.ArgumentList.Add(Path.Combine(repositoryRoot, "private", "Scripts", "Build.py"));
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
            !File.Exists(Path.Combine(directory.FullName, "private", "Scripts", "Build.py")))
        {
            directory = directory.Parent;
        }

        return directory?.FullName ?? throw new DirectoryNotFoundException(
            "Could not locate the NeoWPF repository root.");
    }
}
