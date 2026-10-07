#include <gtest/gtest.h>

#include <NeoWPF/DotNet/Hosting/Host.h>
#include <NeoWPF/DotNet/Hosting/HostOptions.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

namespace {

TEST(DotNetHostTests, ExplicitHostFxrPathHasPriority)
{
    const std::filesystem::path explicitPath = "C:/runtime/hostfxr.dll";
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = explicitPath,
        .dotnetRoot = "C:/dotnet"
    };

    EXPECT_EQ(NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(options), explicitPath);
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

TEST(DotNetHostTests, ManagedBridgeResolvesExecutableRelativeDirectories)
{
    const auto executableDirectory = std::filesystem::temp_directory_path();
    const auto bridgeDirectory = executableDirectory / "NeoWPF.DotNet";

    std::filesystem::create_directory(bridgeDirectory);
    const auto resolved = NeoWPF::DotNet::Hosting::Host::ResolveManagedBridgeDirectory(
        executableDirectory,
        std::filesystem::path());

    EXPECT_EQ(resolved, bridgeDirectory);
    std::filesystem::remove(bridgeDirectory);
}

} // namespace
