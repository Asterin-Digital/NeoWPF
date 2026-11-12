#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>

namespace {

TEST(PaintTests, SolidStoresKindAndColor)
{
    const auto paint = NeoWPF::Rendering::Paint::Solid(0x7F123456u);

    EXPECT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::Solid);
    EXPECT_EQ(paint.color, 0x7F123456u);
}

TEST(PaintTests, IsEmpty_ReturnsTrueForFullyTransparentSolidPaint)
{
    const auto paint = NeoWPF::Rendering::Paint::Solid(0x00000000u);

    EXPECT_TRUE(paint.IsEmpty());
}

TEST(PaintTests, IsEmpty_ReturnsFalseForPartiallyTransparentSolidPaint)
{
    const auto paint = NeoWPF::Rendering::Paint::Solid(0x01000000u);

    EXPECT_FALSE(paint.IsEmpty());
}

TEST(PaintTests, IsEmpty_ReturnsFalseForOpaqueSolidPaint)
{
    const auto paint = NeoWPF::Rendering::Paint::Solid(0xFF000000u);

    EXPECT_FALSE(paint.IsEmpty());
}

}