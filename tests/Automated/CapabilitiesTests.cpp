#include <gtest/gtest.h>
#include <NeoWPF/Platform/Capabilities.h>

#include <array>

namespace {

using NeoWPF::Platform::Capabilities::Capability;
using NeoWPF::Platform::Capabilities::CapabilityGroup;
using NeoWPF::Platform::Capabilities::CapabilityStatus;
using NeoWPF::Platform::Capabilities::CapabilitySupport;

constexpr std::array<Capability, 3> Features{{
    {"vulkan", CapabilitySupport::Supported},
    {"d3d11", CapabilitySupport::Unsupported},
    {"input", CapabilitySupport::Supported},
}};

constexpr CapabilityGroup Group{"rendering", Features, 2};
constexpr CapabilityGroup EmptyGroup{"empty", {}, 0};

}

TEST(CapabilitiesTests, FindAndStatusDistinguishDeclaredAndMissingNames)
{
    using NeoWPF::Platform::Capabilities::Find;
    using NeoWPF::Platform::Capabilities::GetStatus;
    using NeoWPF::Platform::Capabilities::IsDeclared;

    ASSERT_NE(Find(Group, "vulkan"), nullptr);
    EXPECT_EQ(GetStatus(*Find(Group, "vulkan")), CapabilityStatus::Supported);
    EXPECT_EQ(GetStatus(*Find(Group, "d3d11")), CapabilityStatus::Unsupported);
    EXPECT_EQ(Find(Group, "metal"), nullptr);
    EXPECT_EQ(GetStatus(Group, "metal"), CapabilityStatus::Undeclared);
    EXPECT_TRUE(IsDeclared(Group, "vulkan"));
    EXPECT_FALSE(IsDeclared(Group, "metal"));
}

TEST(CapabilitiesTests, GroupQueriesReflectSupportedAndUnsupportedEntries)
{
    using NeoWPF::Platform::Capabilities::AreAllSupported;
    using NeoWPF::Platform::Capabilities::CountSupported;
    using NeoWPF::Platform::Capabilities::CountUnsupported;
    using NeoWPF::Platform::Capabilities::HasAnySupported;
    using NeoWPF::Platform::Capabilities::IsSupported;
    using NeoWPF::Platform::Capabilities::IsUnsupported;

    EXPECT_TRUE(IsSupported(Group, "vulkan"));
    EXPECT_FALSE(IsSupported(Group, "d3d11"));
    EXPECT_FALSE(IsSupported(Group, "metal"));
    EXPECT_TRUE(IsUnsupported(Group, "d3d11"));
    EXPECT_FALSE(IsUnsupported(Group, "metal"));
    EXPECT_EQ(CountSupported(Group), 2u);
    EXPECT_EQ(CountUnsupported(Group), 1u);
    EXPECT_TRUE(HasAnySupported(Group));
    EXPECT_FALSE(AreAllSupported(Group));
}

TEST(CapabilitiesTests, EmptyGroupsHaveNoSupportedEntries)
{
    using NeoWPF::Platform::Capabilities::AreAllSupported;
    using NeoWPF::Platform::Capabilities::CountSupported;
    using NeoWPF::Platform::Capabilities::CountUnsupported;
    using NeoWPF::Platform::Capabilities::HasAnySupported;

    EXPECT_EQ(CountSupported(EmptyGroup), 0u);
    EXPECT_EQ(CountUnsupported(EmptyGroup), 0u);
    EXPECT_FALSE(HasAnySupported(EmptyGroup));
    EXPECT_TRUE(AreAllSupported(EmptyGroup));
}

TEST(CapabilitiesTests, SupportThresholdChecksBoundaries)
{
    using NeoWPF::Platform::Capabilities::HasAtLeastSupported;
    using NeoWPF::Platform::Capabilities::MeetsRequirement;

    EXPECT_TRUE(HasAtLeastSupported(Group, 0));
    EXPECT_TRUE(HasAtLeastSupported(Group, 2));
    EXPECT_FALSE(HasAtLeastSupported(Group, 3));
    EXPECT_FALSE(HasAtLeastSupported(Group, 4));
    EXPECT_TRUE(MeetsRequirement(Group));

    constexpr CapabilityGroup TooHighRequirement{"too-high", Features, 4};
    constexpr CapabilityGroup EmptyRequirement{"empty-requirement", {}, 1};
    EXPECT_FALSE(MeetsRequirement(TooHighRequirement));
    EXPECT_FALSE(MeetsRequirement(EmptyRequirement));
}