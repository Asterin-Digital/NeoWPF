#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>

#include <algorithm>
#include <vector>

namespace {

TEST(PolygonClipTests, RectClipInterpolatesUvs)
{
    const NeoWPF::Rendering::ClipRect clip{25.0, 25.0, 50.0, 50.0, true};
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateRect(vertices, 0.0, 0.0, 100.0, 100.0, clip);

    ASSERT_FALSE(vertices.empty());
    for (const auto& vertex : vertices) {
        EXPECT_GE(vertex.x, clip.x);
        EXPECT_LE(vertex.x, clip.x + clip.w);
        EXPECT_GE(vertex.y, clip.y);
        EXPECT_LE(vertex.y, clip.y + clip.h);
        EXPECT_FLOAT_EQ(vertex.u, static_cast<float>(vertex.x / 100.0));
        EXPECT_FLOAT_EQ(vertex.v, static_cast<float>(vertex.y / 100.0));
    }
}

TEST(PolygonClipTests, RoundedClipKeepsCornerAnchor)
{
    const NeoWPF::Rendering::ClipRect clip{10.0, 0.0, 90.0, 100.0, true};
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateRoundedRect(
        vertices, 0.0, 0.0, 100.0, 100.0, 20.0, 20.0, 20.0, 20.0, clip);

    ASSERT_FALSE(vertices.empty());
    double firstTopVertexX = 100.0;
    for (const auto& vertex : vertices) {
        EXPECT_GE(vertex.x, clip.x);
        EXPECT_LE(vertex.x, clip.x + clip.w);
        EXPECT_GE(vertex.y, clip.y);
        EXPECT_LE(vertex.y, clip.y + clip.h);
        if (vertex.y == 0.0) {
            firstTopVertexX = (std::min)(firstTopVertexX, vertex.x);
        }
    }
    EXPECT_DOUBLE_EQ(firstTopVertexX, 20.0);
}

TEST(PolygonClipTests, ClipsRotatedRectInWorldSpace)
{
    const NeoWPF::Rendering::ClipRect clip{0.0, 0.0, 40.0, 40.0, true};
    std::vector<NeoWPF::Rendering::GeometryVertex> vertices;
    NeoWPF::Rendering::TessellateRect(vertices, 10.0, 10.0, 30.0, 30.0,
                                      clip, 0.5f, 25.0, 25.0);

    ASSERT_FALSE(vertices.empty());
    ASSERT_EQ(vertices.size() % 3, 0u);
    for (const auto& vertex : vertices) {
        EXPECT_GE(vertex.x, clip.x);
        EXPECT_LE(vertex.x, clip.x + clip.w);
        EXPECT_GE(vertex.y, clip.y);
        EXPECT_LE(vertex.y, clip.y + clip.h);
    }
}

}