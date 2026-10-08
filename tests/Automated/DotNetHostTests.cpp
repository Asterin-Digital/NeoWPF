#include <gtest/gtest.h>

#include <NeoWPF/DotNet/Hosting/Host.h>
#include <NeoWPF/DotNet/Hosting/HostOptions.h>
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
