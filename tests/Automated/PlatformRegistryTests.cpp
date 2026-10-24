#include <gtest/gtest.h>

#include <NeoWPF/Platform/PlatformRegistry.h>

#include <utility>

namespace {

class PlatformRegistryReset final {
public:
    ~PlatformRegistryReset()
    {
        NeoWPF::Platform::RegisterPlatform(nullptr);
    }
};

TEST(PlatformRegistryTests, RegistersAndClearsDefaultPlatform)
{
    PlatformRegistryReset reset;
    NeoWPF::Platform::RegisterPlatform(nullptr);

    EXPECT_FALSE(NeoWPF::Platform::HasPlatform());

    auto platform = NeoWPF::Platform::CreateDefaultPlatform();
    ASSERT_NE(platform, nullptr);
    NeoWPF::Platform::RegisterPlatform(std::move(platform));

    EXPECT_TRUE(NeoWPF::Platform::HasPlatform());
    EXPECT_NE(NeoWPF::Platform::CurrentPlatform(), nullptr);

    NeoWPF::Platform::RegisterPlatform(nullptr);
    EXPECT_FALSE(NeoWPF::Platform::HasPlatform());
}

}
