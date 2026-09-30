#include <gtest/gtest.h>

#if defined(_WIN32)
#include <Windows.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Rendering/Clipping.h>
#include <NeoWPF/Rendering/Renderer.h>

#if !defined(_WIN32)
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>
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

#if defined(_WIN32)

TEST(WindowRenderLifecycleTests, CreatesTwoIndependentWindowsAndBindsEachRenderTargetSeparately)
{
    NeoWPF::Core::Application::Initialize();
    auto& app = NeoWPF::Core::Application::Current();

    auto leftWindow = app.CreateWindow(L"NeoWPF window smoke left", 320, 240);
    auto rightWindow = app.CreateWindow(L"NeoWPF window smoke right", 640, 480);

    ASSERT_NE(leftWindow.Handle(), nullptr);
    ASSERT_NE(rightWindow.Handle(), nullptr);
    EXPECT_NE(leftWindow.Handle(), rightWindow.Handle());

    ::ShowWindow(static_cast<HWND>(leftWindow.Handle()), SW_HIDE);
    ::ShowWindow(static_cast<HWND>(rightWindow.Handle()), SW_HIDE);

    NeoWPF::Rendering::Renderer_Initialize(leftWindow.Handle());
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_IsInitialized());

    NeoWPF::Rendering::Renderer_BindWindow(leftWindow.Handle());
    NeoWPF::Rendering::Renderer_Resize(320, 240);
    NeoWPF::Rendering::CommandList leftCommands;
    leftCommands.DrawRect(10.0, 10.0, 100.0, 80.0, 0xFF336699u);
    NeoWPF::Rendering::Renderer_BeginFrame();
    NeoWPF::Rendering::Renderer_Execute(leftCommands);
    NeoWPF::Rendering::Renderer_Present();

    NeoWPF::Rendering::Renderer_BindWindow(rightWindow.Handle());
    NeoWPF::Rendering::Renderer_Resize(640, 480);
    NeoWPF::Rendering::CommandList rightCommands;
    rightCommands.DrawRect(20.0, 20.0, 200.0, 160.0, 0xFF993366u);
    NeoWPF::Rendering::Renderer_BeginFrame();
    NeoWPF::Rendering::Renderer_Execute(rightCommands);
    NeoWPF::Rendering::Renderer_Present();

    NeoWPF::Rendering::Renderer_ReleaseWindow(rightWindow.Handle());
    NeoWPF::Rendering::Renderer_ReleaseWindow(leftWindow.Handle());
    NeoWPF::Rendering::Renderer_Shutdown();
    NeoWPF::Core::Application::Shutdown();
}

#endif

#if !defined(_WIN32)

TEST(VulkanRenderingTests, RequiresPremultipliedCompositeAlphaForTransparentWindows)
{
    using NeoWPF::Rendering::Vulkan::SelectSurfaceCompositeAlpha;

    const auto transparent = SelectSurfaceCompositeAlpha(
        true, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
              VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
    ASSERT_TRUE(transparent.has_value());
    EXPECT_EQ(*transparent, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);

    EXPECT_FALSE(SelectSurfaceCompositeAlpha(
        true, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR).has_value());
    const auto opaque = SelectSurfaceCompositeAlpha(
        false, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
               VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
    ASSERT_TRUE(opaque.has_value());
    EXPECT_EQ(*opaque, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
}

TEST(VulkanRenderingTests, PremultipliesOnlyTransparentClearColors)
{
    using NeoWPF::Rendering::Vulkan::ToSurfaceClearColor;

    const auto premultiplied = ToSurfaceClearColor(0x80402010u, true);
    EXPECT_NEAR(premultiplied[0], (64.0f / 255.0f) * (128.0f / 255.0f), 0.000001f);
    EXPECT_NEAR(premultiplied[1], (32.0f / 255.0f) * (128.0f / 255.0f), 0.000001f);
    EXPECT_NEAR(premultiplied[2], (16.0f / 255.0f) * (128.0f / 255.0f), 0.000001f);
    EXPECT_FLOAT_EQ(premultiplied[3], 128.0f / 255.0f);

    const auto opaque = ToSurfaceClearColor(0x80402010u, false);
    EXPECT_FLOAT_EQ(opaque[0], 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(opaque[1], 32.0f / 255.0f);
    EXPECT_FLOAT_EQ(opaque[2], 16.0f / 255.0f);
    EXPECT_FLOAT_EQ(opaque[3], 128.0f / 255.0f);
}

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