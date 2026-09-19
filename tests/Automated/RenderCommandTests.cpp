#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Clipping.h>
#include <NeoWPF/Rendering/Renderer.h>

#if !defined(_WIN32)
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>
#endif

namespace {

TEST(RenderCommandTests, PreservesCommandOrderAndPayload)
{
    NeoWPF::Rendering::CommandList commands;
    commands.DrawRect(1.0f, 2.0f, 30.0f, 40.0f, 0xFF112233u);
    commands.DrawRoundedRect(3.0f, 4.0f, 20.0f, 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 0xFF445566u);
    commands.DrawTexturedQuad(5.0f, 6.0f, 7.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 42u, 0xFFFFFFFFu);

    ASSERT_EQ(commands.commands.size(), 3u);
    EXPECT_EQ(commands.commands[0].kind, NeoWPF::Rendering::CommandList::CommandKind::Rect);
    EXPECT_EQ(commands.commands[1].kind, NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
    EXPECT_EQ(commands.commands[2].kind, NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad);
    EXPECT_DOUBLE_EQ(commands.commands[0].rect.x, 1.0);
    EXPECT_DOUBLE_EQ(commands.commands[1].roundedRect.bottomLeft, 4.0);
    EXPECT_EQ(commands.commands[2].texturedQuad.textureId, 42u);
}

TEST(RenderCommandTests, ClearRemovesAllQueuedCommands)
{
    NeoWPF::Rendering::CommandList commands;
    commands.DrawRect(0.0f, 0.0f, 1.0f, 1.0f, 0xFFFFFFFFu);
    commands.Clear();

    EXPECT_TRUE(commands.commands.empty());
}

TEST(RenderCommandTests, ClipsRectangleAndReportsVisibleIntersection)
{
    const NeoWPF::Rendering::ClipRect clip{10.0f, 20.0f, 30.0f, 40.0f, true};
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    const bool clipped = NeoWPF::Rendering::ClipRectToBounds(
        0.0f, 10.0f, 50.0f, 30.0f, clip, x, y, width, height);

    EXPECT_TRUE(clipped);
    EXPECT_DOUBLE_EQ(x, 10.0);
    EXPECT_DOUBLE_EQ(y, 20.0);
    EXPECT_DOUBLE_EQ(width, 30.0);
    EXPECT_DOUBLE_EQ(height, 20.0);
}

#if !defined(_WIN32)

TEST(VulkanRenderingTests, RejectsInvalidWaylandWindowHandle)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;

    EXPECT_FALSE(rendering.Initialize(nullptr));
    EXPECT_FALSE(rendering.IsInitialized());
}

TEST(VulkanRenderingTests, ShutdownIsIdempotentBeforeInitialization)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;

    rendering.Shutdown();
    rendering.Shutdown();

    EXPECT_FALSE(rendering.IsInitialized());
}

TEST(VulkanRenderingTests, FrameAndTextureOperationsAreSafeBeforeInitialization)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;
    NeoWPF::Rendering::CommandList commands;

    rendering.BeginFrame();
    rendering.Execute(commands);
    rendering.Present();
    rendering.DestroyTexture(1);
    rendering.ClearTextures();

    EXPECT_EQ(rendering.CreateTexture(1, 1, nullptr), 0u);
    EXPECT_FALSE(rendering.IsInitialized());
}

#endif

}