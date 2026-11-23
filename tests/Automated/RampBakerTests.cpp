#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Paint/RampBaker.h>

#include <array>

namespace {

TEST(RampBakerTests, EmptyStopsProduceNoRamp)
{
    std::array<NeoWPF::Rendering::GradientStopDesc, 0> stops{};
    std::array<std::uint32_t, 4> output{};

    NeoWPF::Rendering::BakeRamp(stops, output, output.size());

    for (const auto pixel : output) {
        EXPECT_EQ(pixel, 0u);
    }
}

TEST(RampBakerTests, SingleStopProducesAFlatRamp)
{
    const std::array<NeoWPF::Rendering::GradientStopDesc, 1> stops{{
        {0.0f, 0xFF336699u},
    }};
    std::array<std::uint32_t, 4> output{};

    NeoWPF::Rendering::BakeRamp(stops, output, output.size());

    EXPECT_EQ(output, (std::array<std::uint32_t, 4>{
        0xFF336699u, 0xFF336699u, 0xFF336699u, 0xFF336699u}));
}

TEST(RampBakerTests, TwoStopsBakeEndpointsAndMidpoint)
{
    const std::array<NeoWPF::Rendering::GradientStopDesc, 2> stops{{
        {0.0f, 0xFF000000u},
        {1.0f, 0xFFFFFFFFu},
    }};
    std::array<std::uint32_t, 5> output{};

    NeoWPF::Rendering::BakeRamp(stops, output, output.size());

    EXPECT_EQ(output[0], 0xFF000000u);
    EXPECT_EQ(output[1], 0xFF404040u);
    EXPECT_EQ(output[2], 0xFF808080u);
    EXPECT_EQ(output[3], 0xFFBFBFBFu);
    EXPECT_EQ(output[4], 0xFFFFFFFFu);
}

TEST(RampBakerTests, OutsideRangeUsesStopEndpoints)
{
    const std::array<NeoWPF::Rendering::GradientStopDesc, 2> stops{{
        {0.25f, 0xFFFF0000u},
        {0.75f, 0xFF0000FFu},
    }};
    std::array<std::uint32_t, 3> output{};

    NeoWPF::Rendering::BakeRamp(stops, output, output.size());

    EXPECT_EQ(output[0], 0xFFFF0000u);
    EXPECT_EQ(output[2], 0xFF0000FFu);
}

TEST(RampBakerTests, LinearGalleryColorsProduceDistinctEndpoints)
{
    const std::array<NeoWPF::Rendering::GradientStopDesc, 2> stops{{
        {0.0f, 0xFF3A7BD5u},
        {1.0f, 0xFF8A51DCu},
    }};
    std::array<std::uint32_t, 3> output{};

    NeoWPF::Rendering::BakeRamp(stops, output, output.size());

    EXPECT_EQ(output.front(), 0xFF3A7BD5u);
    EXPECT_EQ(output.back(), 0xFF8A51DCu);
    EXPECT_NE(output.front(), output.back());
}

}
