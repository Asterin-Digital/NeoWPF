#include <gtest/gtest.h>

#include <NeoWPF/DotNet/Hosting/Host.h>
#include <NeoWPF/DotNet/Hosting/HostOptions.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrLibrary.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <array>
#include <fstream>

namespace {

void CreateBridgeFiles(const std::filesystem::path& directory)
{
    std::filesystem::create_directories(directory);
    constexpr std::array<const char*, 3> bridgeFiles{
        "NeoWPF.DotNet.Bridge.dll",
        "NeoWPF.DotNet.Bridge.runtimeconfig.json",
        "NeoWPF.DotNet.Bridge.deps.json"
    };

    for (const auto* file : bridgeFiles)
        std::ofstream(directory / file).put(' ');
}

TEST(DotNetHostTests, ExplicitHostFxrPathHasPriority)
{
    const std::filesystem::path explicitPath = "C:/runtime/hostfxr.dll";
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = explicitPath,
        .dotnetRoot = "C:/dotnet"
    };

    EXPECT_EQ(NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(options), explicitPath);
}

TEST(DotNetHostTests, HostStatusPreservesRawHostFxrResult)
{
    NeoWPF::DotNet::Hosting::HostStatus status(
        NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeInitFailed);

    EXPECT_EQ(status.GetRawHostFxrResult(), 0);
    status.SetRawHostFxrResult(-2147450749);
    EXPECT_EQ(status.GetRawHostFxrResult(), -2147450749);
}

TEST(DotNetHostTests, MissingHostFxrReturnsRuntimeNotFound)
{
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = std::filesystem::path(),
        .dotnetRoot = std::filesystem::path("C:/missing-dotnet")
    };

    const auto status = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPathStatus(options);
    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound);
}

TEST(DotNetHostTests, HostFxrLibraryRejectsInvalidLibraryFile)
{
    const auto hostfxrPath = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.invalid-hostfxr";
    std::ofstream(hostfxrPath).put(' ');

    NeoWPF::DotNet::Hosting::HostFxr::HostFxrLibrary library;
    const auto status = library.Load(hostfxrPath);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrLoadFailed);
    std::filesystem::remove(hostfxrPath);
}

TEST(DotNetHostTests, NethostFindsAndLoadsInstalledHostFxr)
{
    const NeoWPF::DotNet::Hosting::HostOptions options;
    const auto path = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(options);
    if (path.empty())
        GTEST_SKIP() << "No .NET runtime is installed.";

    NeoWPF::DotNet::Hosting::HostFxr::HostFxrLibrary library;
    const auto status = library.Load(path);
    if (!status.Succeeded())
        GTEST_SKIP() << status.Message();

    EXPECT_TRUE(library.HasExport("hostfxr_initialize_for_runtime_config"));
    EXPECT_TRUE(library.HasExport("hostfxr_get_runtime_delegate"));
    EXPECT_TRUE(library.HasExport("hostfxr_close"));
    EXPECT_FALSE(library.HasExport("NeoWPF_Missing_HostFxr_Export"));
}

TEST(DotNetHostTests, FindsCurrentExecutableDirectory)
{
    const auto executableDirectory = NeoWPF::DotNet::Hosting::HostFxr::GetExecutableDirectory();

    ASSERT_FALSE(executableDirectory.empty());
    EXPECT_TRUE(std::filesystem::is_directory(executableDirectory));
}

TEST(DotNetHostTests, HostStartInitializesManagedBridge)
{
    const NeoWPF::DotNet::Hosting::HostOptions defaultOptions;
    const auto hostfxrPath = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(defaultOptions);
    if (hostfxrPath.empty())
        GTEST_SKIP() << "No .NET runtime is installed.";

    const auto bridgeDirectory = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.empty-bridge";
    std::filesystem::create_directories(bridgeDirectory);

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath,
        .bridgeDirectory = bridgeDirectory
    };
    const auto status = host.Start(options);

    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }

    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();
    EXPECT_TRUE(host.Stop());
    std::filesystem::remove_all(bridgeDirectory);
}

TEST(DotNetHostTests, HostStartReportsMissingFrameworkWithRawHostFxrResult)
{
    const auto hostfxrPath = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath({});
    if (hostfxrPath.empty())
        GTEST_SKIP() << "No hostfxr is installed.";

    const auto bridgeDirectory = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.missing-framework";
    CreateBridgeFiles(bridgeDirectory);
    {
        std::ofstream runtimeConfig(bridgeDirectory / "NeoWPF.DotNet.Bridge.runtimeconfig.json");
        runtimeConfig << R"({"runtimeOptions":{"tfm":"net99.0","framework":{"name":"Microsoft.NETCore.App","version":"99.0.0","rollForward":"Disable"}}})";
    }

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath,
        .bridgeDirectory = bridgeDirectory
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
        << status.Message() << " (hostfxr result " << status.GetRawHostFxrResult() << ")";
    EXPECT_NE(status.GetRawHostFxrResult(), 0);
    EXPECT_FALSE(host.Stop());
    std::filesystem::remove_all(bridgeDirectory);
}

TEST(DotNetHostTests, HostStartReportsMissingHostFxrExports)
{
    const auto hostfxrPath = std::filesystem::path("C:/Windows/System32/kernel32.dll");
    if (!std::filesystem::exists(hostfxrPath))
        GTEST_SKIP() << "kernel32.dll is not available on this system.";

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrSymbolMissing)
        << status.Message();
    EXPECT_FALSE(status.Succeeded());
    EXPECT_FALSE(host.Stop());
}

TEST(DotNetHostTests, HostStartDoesNotSucceedFromPathResolutionAlone)
{
    const auto hostfxrPath = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.fake-hostfxr";
    std::ofstream(hostfxrPath).put(' ');

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrLoadFailed);
    EXPECT_FALSE(status.Succeeded());
    EXPECT_FALSE(host.Stop());
    std::filesystem::remove(hostfxrPath);
}

TEST(DotNetHostTests, ManagedBridgeResolvesExecutableRelativeDirectories)
{
    const auto executableDirectory = std::filesystem::temp_directory_path() /
        ("NeoWPF.DotNetHostTests." + std::to_string(std::rand()));
    const auto bridgeDirectory = executableDirectory / "NeoWPF.DotNet";

    CreateBridgeFiles(bridgeDirectory);
    const auto resolved = NeoWPF::DotNet::Hosting::Host::ResolveManagedBridgeDirectory(
        executableDirectory,
        std::filesystem::path());

    EXPECT_EQ(resolved, bridgeDirectory);
    std::filesystem::remove_all(executableDirectory);
}

TEST(DotNetHostTests, ManagedBridgeFallsBackToExecutableDirectory)
{
    const auto executableDirectory = std::filesystem::temp_directory_path() /
        ("NeoWPF.DotNetHostTests." + std::to_string(std::rand()));
    CreateBridgeFiles(executableDirectory);

    const auto resolved = NeoWPF::DotNet::Hosting::Host::ResolveManagedBridgeDirectory(
        executableDirectory,
        std::filesystem::path());

    EXPECT_EQ(resolved, executableDirectory);
    std::filesystem::remove_all(executableDirectory);
}

}
