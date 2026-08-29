#ifndef _WIN32
#include <NeoWPF/Platform/Linux/LImageDecoding.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

const std::vector<std::uint8_t> kOnePixelPng = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
    0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4,
    0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41,
    0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99,
    0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

}

TEST(LImageDecodingTests, DecodesRgbaMemoryAndPreservesAspectRatio)
{
    NeoWPF::Platform::Linux::LImageDecoding decoder;
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;

    ASSERT_TRUE(decoder.DecodeImageFromMemory(kOnePixelPng, pixels, width, height, 0, 2));
    EXPECT_EQ(width, 2);
    EXPECT_EQ(height, 2);
    ASSERT_EQ(pixels.size(), 16u);
    EXPECT_EQ(pixels[0], 255u);
    EXPECT_EQ(pixels[1], 0u);
    EXPECT_EQ(pixels[2], 0u);
    EXPECT_EQ(pixels[3], 255u);
}

TEST(LImageDecodingTests, RejectsEmptyInput)
{
    NeoWPF::Platform::Linux::LImageDecoding decoder;
    std::vector<std::uint8_t> pixels{1, 2, 3};
    int width = 10;
    int height = 10;

    EXPECT_FALSE(decoder.DecodeImageFromMemory({}, pixels, width, height, 0, 0));
    EXPECT_TRUE(pixels.empty());
}
#endif
