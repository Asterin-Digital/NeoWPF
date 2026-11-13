#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace {

TEST(BrushPaintResolutionTests, PreservesColorAtFullOpacity)
{
    NeoWPF::UI::SolidColorBrush brush(0x80446688u);
    NeoWPF::Rendering::CommandList commands;

    const auto paint = brush.ResolvePaint(commands, 1.0, 2.0, 30.0, 40.0);

    EXPECT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::Solid);
    EXPECT_EQ(paint.color, 0x80446688u);
}

TEST(BrushPaintResolutionTests, FoldsBrushOpacityIntoResolvedAlpha)
{
    NeoWPF::UI::SolidColorBrush brush(0xFF112233u);
    brush.SetOpacity(0.5);
    NeoWPF::Rendering::CommandList commands;

    const auto paint = brush.ResolvePaint(commands, 0.0, 0.0, 10.0, 10.0);

    EXPECT_EQ(paint.color, 0x7F112233u);
}

TEST(BrushPaintResolutionTests, ZeroOpacityProducesAnEmptyPaint)
{
    NeoWPF::UI::SolidColorBrush brush(0xFF112233u);
    brush.SetOpacity(0.0);
    NeoWPF::Rendering::CommandList commands;

    const auto paint = brush.ResolvePaint(commands, 0.0, 0.0, 10.0, 10.0);

    EXPECT_TRUE(paint.IsEmpty());
}

}