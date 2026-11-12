#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Geometry/DrawBatcher.h>

namespace {

using NeoWPF::Rendering::CommandList;
using NeoWPF::Rendering::DrawBatchKind;

TEST(DrawBatcherTests, EmptyCommandListProducesNoBatches)
{
    const CommandList commands;

    EXPECT_TRUE(NeoWPF::Rendering::BuildDrawBatches(commands).empty());
}

TEST(DrawBatcherTests, BatchesAdjacentCommandsInOrder)
{
    CommandList commands;
    commands.DrawRect(0.0, 0.0, 10.0, 10.0, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.DrawArc(10.0, 10.0, 5.0, 2.0, 0.0, 1.0, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.DrawTexturedQuad(0.0, 0.0, 10.0, 10.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              7u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.DrawRoundedRect(0.0, 0.0, 10.0, 10.0, 1.0, 1.0, 1.0, 1.0,
                             NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    const auto batches = NeoWPF::Rendering::BuildDrawBatches(commands);

    ASSERT_EQ(batches.size(), 3u);
    EXPECT_EQ(batches[0].kind, DrawBatchKind::Solid);
    EXPECT_EQ(batches[0].firstCommand, 0u);
    EXPECT_EQ(batches[0].commandCount, 2u);
    EXPECT_EQ(batches[1].kind, DrawBatchKind::Textured);
    EXPECT_EQ(batches[1].firstCommand, 2u);
    EXPECT_EQ(batches[1].commandCount, 1u);
    EXPECT_EQ(batches[1].textureId, 7u);
    EXPECT_EQ(batches[2].kind, DrawBatchKind::Solid);
    EXPECT_EQ(batches[2].firstCommand, 3u);
    EXPECT_EQ(batches[2].commandCount, 1u);
}

TEST(DrawBatcherTests, SplitsOnTextureChange)
{
    CommandList commands;
    commands.DrawTexturedQuad(0.0, 0.0, 1.0, 1.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              1u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.DrawTexturedQuad(0.0, 0.0, 1.0, 1.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              1u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.DrawTexturedQuad(0.0, 0.0, 1.0, 1.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              2u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    const auto batches = NeoWPF::Rendering::BuildDrawBatches(commands);

    ASSERT_EQ(batches.size(), 2u);
    EXPECT_EQ(batches[0].kind, DrawBatchKind::Textured);
    EXPECT_EQ(batches[0].firstCommand, 0u);
    EXPECT_EQ(batches[0].commandCount, 2u);
    EXPECT_EQ(batches[0].textureId, 1u);
    EXPECT_EQ(batches[1].kind, DrawBatchKind::Textured);
    EXPECT_EQ(batches[1].firstCommand, 2u);
    EXPECT_EQ(batches[1].commandCount, 1u);
    EXPECT_EQ(batches[1].textureId, 2u);
}

}