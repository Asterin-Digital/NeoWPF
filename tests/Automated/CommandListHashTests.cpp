#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>

namespace {

using NeoWPF::Rendering::CommandList;

TEST(CommandListHashTests, IdenticalCommandListsHaveTheSameHash)
{
    CommandList first;
    first.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112233u));
    first.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                  NeoWPF::Rendering::Paint::Solid(0xFF445566u));

    CommandList second;
    second.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112233u));
    second.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                   NeoWPF::Rendering::Paint::Solid(0xFF445566u));

    EXPECT_EQ(first.ComputeHash(), second.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenCommandPayloadChanges)
{
    CommandList original;
    original.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112234u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenRoundedRectPayloadChanges)
{
    CommandList original;
    original.DrawRoundedRect(1.0, 2.0, 3.0, 4.0, 0.5, 0.5, 0.5, 0.5,
                            NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawRoundedRect(1.0, 2.0, 3.0, 4.0, 0.5, 0.5, 0.75, 0.5,
                            NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenArcPayloadChanges)
{
    CommandList original;
    original.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                     NeoWPF::Rendering::Paint::Solid(0xFF445566u));

    CommandList changed;
    changed.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.75,
                    NeoWPF::Rendering::Paint::Solid(0xFF445566u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenTexturedQuadPayloadChanges)
{
    CommandList original;
    original.DrawTexturedQuad(1.0, 2.0, 3.0, 4.0, 0.0f, 0.0f, 1.0f, 1.0f,
                             10u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    CommandList changed;
    changed.DrawTexturedQuad(1.0, 2.0, 3.0, 4.0, 0.0f, 0.0f, 1.0f, 1.0f,
                             11u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenCommandOrderChanges)
{
    CommandList first;
    first.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112233u));
    first.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                  NeoWPF::Rendering::Paint::Solid(0xFF445566u));

    CommandList second;
    second.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                   NeoWPF::Rendering::Paint::Solid(0xFF445566u));
    second.DrawRect(1.0, 2.0, 3.0, 4.0, NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    EXPECT_NE(first.ComputeHash(), second.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenRectPaintChanges)
{
    CommandList original;
    original.DrawRect(1.0, 2.0, 3.0, 4.0,
                      NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawRect(1.0, 2.0, 3.0, 4.0,
                     NeoWPF::Rendering::Paint::Solid(0xFF112234u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenRoundedRectPaintChanges)
{
    CommandList original;
    original.DrawRoundedRect(1.0, 2.0, 3.0, 4.0, 0.5, 0.5, 0.5, 0.5,
                             NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawRoundedRect(1.0, 2.0, 3.0, 4.0, 0.5, 0.5, 0.5, 0.5,
                            NeoWPF::Rendering::Paint::Solid(0xFF112234u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenArcPaintChanges)
{
    CommandList original;
    original.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                     NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawArc(10.0, 11.0, 12.0, 2.0, 0.25, 1.5,
                    NeoWPF::Rendering::Paint::Solid(0xFF112234u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenTexturedQuadTintChanges)
{
    CommandList original;
    original.DrawTexturedQuad(1.0, 2.0, 3.0, 4.0, 0.0f, 0.0f, 1.0f, 1.0f,
                              10u, NeoWPF::Rendering::Paint::Solid(0xFF112233u));

    CommandList changed;
    changed.DrawTexturedQuad(1.0, 2.0, 3.0, 4.0, 0.0f, 0.0f, 1.0f, 1.0f,
                             10u, NeoWPF::Rendering::Paint::Solid(0xFF112234u));

    EXPECT_NE(original.ComputeHash(), changed.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenClearColorChanges)
{
    CommandList first;
    first.clearColor = 0xFF112233u;

    CommandList second;
    second.clearColor = 0xFF112234u;

    EXPECT_NE(first.ComputeHash(), second.ComputeHash());
}

TEST(CommandListHashTests, HashChangesWhenClearColorPresenceChanges)
{
    CommandList first;
    first.clearColor = 0xFF112233u;
    first.hasClearColor = false;

    CommandList second;
    second.clearColor = 0xFF112233u;
    second.hasClearColor = true;

    EXPECT_NE(first.ComputeHash(), second.ComputeHash());
}

}