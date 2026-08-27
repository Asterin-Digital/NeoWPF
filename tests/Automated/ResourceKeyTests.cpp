#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <cstdint>
#include <random>
#include <string>

namespace {

TEST(ResourceKeyTests, PreservesNameAndProducesStableHash)
{
    constexpr NeoWPF::ResourceKey key("Control.Background");
    constexpr NeoWPF::ResourceKey sameKey("Control.Background");

    EXPECT_STREQ(key.Name(), "Control.Background");
    EXPECT_EQ(key.Hash(), sameKey.Hash());
    EXPECT_NE(key.Hash(), NeoWPF::ResourceKey("Control.Foreground").Hash());
}

TEST(ResourceKeyTests, UsesFNV1aHashForKnownValues)
{
    constexpr NeoWPF::ResourceKey emptyKey("");
    constexpr NeoWPF::ResourceKey key("hello");

    EXPECT_EQ(emptyKey.Hash(), 14695981039346656037ull);
    EXPECT_EQ(key.Hash(), 11831194018420276491ull);
}

TEST(ResourceKeyTests, SupportsUnicodeAndLongNames)
{
    const std::string longName(1024, 'x');
    const NeoWPF::ResourceKey unicodeKey("Theme.A9");
    const NeoWPF::ResourceKey longKey(longName.c_str());

    EXPECT_STREQ(unicodeKey.Name(), "Theme.A9");
    EXPECT_EQ(longKey.Name(), longName.c_str());
    EXPECT_NE(unicodeKey.Hash(), longKey.Hash());
}

TEST(ResourceKeyTests, MatchesIndependentHashOracleForRandomNames)
{
    std::mt19937_64 generator(0x4E656F575046ULL);
    std::uniform_int_distribution<size_t> lengthDistribution(0, 512);
    std::uniform_int_distribution<unsigned int> byteDistribution(1, 255);

    for (int iteration = 0; iteration < 10000; ++iteration) {
        const size_t length = lengthDistribution(generator);
        std::string name(length, '\0');
        for (char& character : name) {
            character = static_cast<char>(byteDistribution(generator));
        }

        std::uint64_t expectedHash = 14695981039346656037ull;
        for (const unsigned char character : name) {
            expectedHash ^= character;
            expectedHash *= 1099511628211ull;
        }

        const NeoWPF::ResourceKey key(name.c_str());
        EXPECT_EQ(key.Hash(), expectedHash) << "iteration " << iteration;
        EXPECT_EQ(std::string(key.Name()), name) << "iteration " << iteration;
    }
}

TEST(ResourceKeyTests, HandlesBoundaryLengthsAndEmbeddedNulls)
{
    for (const size_t length : {0u, 1u, 2u, 255u, 256u, 1024u, 4096u}) {
        const std::string name(length, 'a');
        const NeoWPF::ResourceKey key(name.c_str());
        EXPECT_EQ(std::string(key.Name()), name) << "length " << length;
    }

    const std::string nameWithNull("prefix\0suffix", 13);
    const NeoWPF::ResourceKey key(nameWithNull.c_str());
    EXPECT_STREQ(key.Name(), "prefix");
}

TEST(ResourceKeyTests, PreservesMalformedByteSequencesWithoutReencoding)
{
    std::string malformed;
    malformed.reserve(256);
    for (unsigned int byte = 1; byte <= 255; ++byte) {
        malformed.push_back(static_cast<char>(byte));
    }

    const NeoWPF::ResourceKey key(malformed.c_str());
    std::uint64_t expectedHash = 14695981039346656037ull;
    for (const unsigned char byte : malformed) {
        expectedHash ^= byte;
        expectedHash *= 1099511628211ull;
    }

    EXPECT_EQ(std::string(key.Name()), malformed);
    EXPECT_EQ(key.Hash(), expectedHash);
}

TEST(ResourceKeyTests, HandlesHostileRepeatedAndAlternatingBytes)
{
    std::string hostile;
    hostile.reserve(8192);
    for (int index = 0; index < 4096; ++index) {
        hostile.push_back((index % 2 == 0) ? '\x01' : '\xFF');
    }

    const NeoWPF::ResourceKey key(hostile.c_str());
    EXPECT_EQ(std::string(key.Name()), hostile);
    EXPECT_NE(key.Hash(), NeoWPF::ResourceKey("\x01").Hash());
}

TEST(ResourceKeyTests, MacroCreatesResourceKey)
{
    constexpr auto key = NEOWPF_RESOURCE_KEY("Application.Theme");

    EXPECT_STREQ(key.Name(), "Application.Theme");
    EXPECT_EQ(key.Hash(), NeoWPF::ResourceKey("Application.Theme").Hash());
}

TEST(PropertyFlagsTests, SupportsCombiningAndCheckingFlags)
{
    using NeoWPF::UI::Properties::HasFlag;
    using NeoWPF::UI::Properties::PropertyFlags;

    const auto flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;

    EXPECT_TRUE(HasFlag(flags, PropertyFlags::AffectsMeasure));
    EXPECT_TRUE(HasFlag(flags, PropertyFlags::AffectsRender));
    EXPECT_FALSE(HasFlag(flags, PropertyFlags::AffectsArrange));
}

TEST(PropertyFlagsTests, HandlesNoneAndAllFlags)
{
    using NeoWPF::UI::Properties::HasFlag;
    using NeoWPF::UI::Properties::PropertyFlags;

    const auto allFlags = PropertyFlags::AffectsMeasure |
                          PropertyFlags::AffectsArrange |
                          PropertyFlags::AffectsRender |
                          PropertyFlags::Inherits;

    EXPECT_FALSE(HasFlag(PropertyFlags::None, PropertyFlags::None));
    EXPECT_TRUE(HasFlag(allFlags, PropertyFlags::AffectsMeasure));
    EXPECT_TRUE(HasFlag(allFlags, PropertyFlags::AffectsArrange));
    EXPECT_TRUE(HasFlag(allFlags, PropertyFlags::AffectsRender));
    EXPECT_TRUE(HasFlag(allFlags, PropertyFlags::Inherits));
}

TEST(PropertyFlagsTests, SupportsCompoundAssignmentAndIntersection)
{
    using NeoWPF::UI::Properties::HasFlag;
    using NeoWPF::UI::Properties::PropertyFlags;

    auto flags = PropertyFlags::AffectsMeasure;
    flags |= PropertyFlags::AffectsRender;

    EXPECT_EQ(flags & PropertyFlags::AffectsMeasure, PropertyFlags::AffectsMeasure);
    EXPECT_EQ(flags & PropertyFlags::AffectsArrange, PropertyFlags::None);
    EXPECT_TRUE(HasFlag(flags, PropertyFlags::AffectsRender));
}

TEST(PropertyFlagsTests, HandlesEveryBitmaskCombination)
{
    using NeoWPF::UI::Properties::HasFlag;
    using NeoWPF::UI::Properties::PropertyFlags;

    constexpr PropertyFlags flags[] = {
        PropertyFlags::AffectsMeasure,
        PropertyFlags::AffectsArrange,
        PropertyFlags::AffectsRender,
        PropertyFlags::Inherits,
    };

    for (std::uint32_t mask = 0; mask < 16; ++mask) {
        const auto combination = static_cast<PropertyFlags>(mask);
        for (std::uint32_t bit = 0; bit < 4; ++bit) {
            const bool expected = (mask & (1u << bit)) != 0;
            EXPECT_EQ(HasFlag(combination, flags[bit]), expected)
                << "mask " << mask << ", bit " << bit;
        }
    }
}

TEST(PropertyFlagsTests, IgnoresUnknownBitsWhenCheckingKnownFlags)
{
    using NeoWPF::UI::Properties::HasFlag;
    using NeoWPF::UI::Properties::PropertyFlags;

    const auto malformedFlags = static_cast<PropertyFlags>(0xFFFFFFFFu);
    const auto unknownOnly = static_cast<PropertyFlags>(0xFFFFFFF0u);

    EXPECT_TRUE(HasFlag(malformedFlags, PropertyFlags::AffectsMeasure));
    EXPECT_TRUE(HasFlag(malformedFlags, PropertyFlags::AffectsArrange));
    EXPECT_TRUE(HasFlag(malformedFlags, PropertyFlags::AffectsRender));
    EXPECT_TRUE(HasFlag(malformedFlags, PropertyFlags::Inherits));
    EXPECT_FALSE(HasFlag(unknownOnly, PropertyFlags::AffectsMeasure));
    EXPECT_FALSE(HasFlag(unknownOnly, PropertyFlags::AffectsArrange));
}

}