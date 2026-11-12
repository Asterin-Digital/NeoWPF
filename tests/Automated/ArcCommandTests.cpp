#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Geometry/ArcTessellation.h>
#include <NeoWPF/Rendering/Renderer.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;

TEST(ArcCommandTests, PreservesCommandOrderAndArcPayload)
{
    NeoWPF::Rendering::CommandList commands;
    const NeoWPF::Rendering::ClipRect clip{10.0, 20.0, 30.0, 40.0, true};
    commands.DrawRect(1.0, 2.0, 3.0, 4.0,
                      NeoWPF::Rendering::Paint::Solid(0xFF112233u));
    commands.DrawArc(50.0, 60.0, 12.0, 4.0, 0.25, 1.5,
                     NeoWPF::Rendering::Paint::Solid(0xFF445566u), 0.75f, clip);
    commands.DrawTexturedQuad(5.0, 6.0, 7.0, 8.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              42u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    ASSERT_EQ(commands.commands.size(), 3u);
    EXPECT_EQ(commands.commands[0].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::Rect);
    EXPECT_EQ(commands.commands[1].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::Arc);
    EXPECT_EQ(commands.commands[2].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad);

    const auto& arc = commands.commands[1].GetArc();
    EXPECT_DOUBLE_EQ(arc.centerX, 50.0);
    EXPECT_DOUBLE_EQ(arc.centerY, 60.0);
    EXPECT_DOUBLE_EQ(arc.radius, 12.0);
    EXPECT_DOUBLE_EQ(arc.thickness, 4.0);
    EXPECT_DOUBLE_EQ(arc.startAngleRadians, 0.25);
    EXPECT_DOUBLE_EQ(arc.sweepAngleRadians, 1.5);
    EXPECT_EQ(arc.paint.color, 0xFF445566u);
    EXPECT_FLOAT_EQ(arc.opacity, 0.75f);
    EXPECT_DOUBLE_EQ(arc.clip.x, 10.0);
    EXPECT_DOUBLE_EQ(arc.clip.y, 20.0);
    EXPECT_DOUBLE_EQ(arc.clip.w, 30.0);
    EXPECT_DOUBLE_EQ(arc.clip.h, 40.0);
    EXPECT_TRUE(arc.clip.active);
}

TEST(ArcCommandTests, TessellatesPositiveQuarterSweepClockwiseFromTop)
{
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateAnnularArc(
        vertices, 50.0, 50.0, 10.0, 2.0, 0.0, kPi * 0.5, {});

    ASSERT_FALSE(vertices.empty());
    ASSERT_EQ(vertices.size() % 3, 0u);
    double minimumX = vertices.front().x;
    double maximumX = vertices.front().x;
    double minimumY = vertices.front().y;
    double maximumY = vertices.front().y;
    for (const auto& vertex : vertices) {
        minimumX = (std::min)(minimumX, vertex.x);
        maximumX = (std::max)(maximumX, vertex.x);
        minimumY = (std::min)(minimumY, vertex.y);
        maximumY = (std::max)(maximumY, vertex.y);
    }

    EXPECT_NEAR(minimumX, 50.0, 1e-9);
    EXPECT_NEAR(maximumX, 61.0, 1e-9);
    EXPECT_NEAR(minimumY, 39.0, 1e-9);
    EXPECT_NEAR(maximumY, 50.0, 1e-9);
}

TEST(ArcCommandTests, ClampsSweepsToOneRevolution)
{
    std::vector<NeoWPF::Rendering::GeometryVertex> oneRevolution;
    std::vector<NeoWPF::Rendering::GeometryVertex> twoRevolutions;
    NeoWPF::Rendering::TessellateAnnularArc(
        oneRevolution, 0.0, 0.0, 10.0, 2.0, 0.0, 2.0 * kPi, {});
    NeoWPF::Rendering::TessellateAnnularArc(
        twoRevolutions, 0.0, 0.0, 10.0, 2.0, 0.0, 4.0 * kPi, {});

    ASSERT_FALSE(oneRevolution.empty());
    EXPECT_EQ(twoRevolutions.size(), oneRevolution.size());
}

TEST(ArcCommandTests, ReturnsEmptyGeometryForInvalidOrZeroSweep)
{
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateAnnularArc(vertices, 0.0, 0.0, 0.0, 2.0, 0.0, kPi, {});
    EXPECT_TRUE(vertices.empty());
    NeoWPF::Rendering::TessellateAnnularArc(vertices, 0.0, 0.0, 10.0, 0.0, 0.0, kPi, {});
    EXPECT_TRUE(vertices.empty());
    NeoWPF::Rendering::TessellateAnnularArc(vertices, 0.0, 0.0, 10.0, 21.0, 0.0, kPi, {});
    EXPECT_TRUE(vertices.empty());
    NeoWPF::Rendering::TessellateAnnularArc(vertices, 0.0, 0.0, 10.0, 2.0, 0.0, 0.0, {});
    EXPECT_TRUE(vertices.empty());
    NeoWPF::Rendering::TessellateAnnularArc(
        vertices, std::numeric_limits<double>::infinity(), 0.0, 10.0, 2.0, 0.0, kPi, {});
    EXPECT_TRUE(vertices.empty());
}

TEST(ArcCommandTests, ClipsGeneratedTrianglesToClipBounds)
{
    const NeoWPF::Rendering::ClipRect clip{8.0, -5.0, 4.0, 5.0, true};
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateAnnularArc(
        vertices, 0.0, 0.0, 10.0, 2.0, 0.0, kPi * 0.5, clip);

    ASSERT_FALSE(vertices.empty());
    ASSERT_EQ(vertices.size() % 3, 0u);
    for (const auto& vertex : vertices) {
        EXPECT_GE(vertex.x, clip.x - 1e-9);
        EXPECT_LE(vertex.x, clip.x + clip.w + 1e-9);
        EXPECT_GE(vertex.y, clip.y - 1e-9);
        EXPECT_LE(vertex.y, clip.y + clip.h + 1e-9);
    }
}

TEST(ArcCommandTests, ReturnsEmptyGeometryWhenClipExcludesArc)
{
    const NeoWPF::Rendering::ClipRect clip{20.0, 20.0, 5.0, 5.0, true};
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateAnnularArc(
        vertices, 0.0, 0.0, 10.0, 2.0, 0.0, kPi * 0.5, clip);
    EXPECT_TRUE(vertices.empty());
}

}