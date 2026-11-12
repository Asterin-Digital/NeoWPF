#include <gtest/gtest.h>

#if defined(_WIN32)
#include <Windows.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Rendering/Geometry/Clipping.h>
#include <NeoWPF/Rendering/Renderer.h>

#if !defined(_WIN32)
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>
#endif

namespace {

TEST(RenderCommandTests, PreservesCommandOrderAndPayload)
{
    NeoWPF::Rendering::CommandList commands;
    commands.DrawRect(1.0f, 2.0f, 30.0f, 40.0f,
                      NeoWPF::Rendering::Paint::Solid(0xFF112233u));
    commands.DrawRoundedRect(3.0f, 4.0f, 20.0f, 10.0f, 1.0f, 2.0f, 3.0f, 4.0f,
                             NeoWPF::Rendering::Paint::Solid(0xFF445566u));
    commands.DrawTexturedQuad(5.0f, 6.0f, 7.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                              42u, NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));

    ASSERT_EQ(commands.commands.size(), 3u);
    EXPECT_EQ(commands.commands[0].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::Rect);
    EXPECT_EQ(commands.commands[1].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::RoundedRect);
    EXPECT_EQ(commands.commands[2].GetKind(), NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad);
    EXPECT_DOUBLE_EQ(commands.commands[0].GetRect().x, 1.0);
    EXPECT_DOUBLE_EQ(commands.commands[1].GetRoundedRect().bottomLeft, 4.0);
    EXPECT_EQ(commands.commands[2].GetTexturedQuad().textureId, 42u);
}

TEST(RenderCommandTests, ClearRemovesAllQueuedCommands)
{
    NeoWPF::Rendering::CommandList commands;
    commands.DrawRect(0.0f, 0.0f, 1.0f, 1.0f,
                      NeoWPF::Rendering::Paint::Solid(0xFFFFFFFFu));
    commands.Clear();

    EXPECT_TRUE(commands.commands.empty());
}

TEST(RenderCommandTests, StoresOnlyOnePayload)
{
    using Command = NeoWPF::Rendering::CommandList::Command;
    using CommandList = NeoWPF::Rendering::CommandList;
    constexpr std::size_t oldInlinePayloadSize = sizeof(CommandList::RectData) +
        sizeof(CommandList::RoundedRectData) + sizeof(CommandList::ArcData) +
        sizeof(CommandList::TexturedQuadData);

    EXPECT_LT(sizeof(Command), oldInlinePayloadSize);
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

    const NeoWPF::Rendering::RenderSurface leftSurface{leftWindow.Handle()};
    const NeoWPF::Rendering::RenderSurface rightSurface{rightWindow.Handle()};
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_Initialize(leftSurface));
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_IsInitialized());

    NeoWPF::Rendering::Renderer_Resize(leftSurface, 320, 240);
    NeoWPF::Rendering::CommandList leftCommands;
    leftCommands.DrawRect(10.0, 10.0, 100.0, 80.0,
                          NeoWPF::Rendering::Paint::Solid(0xFF336699u));
    NeoWPF::Rendering::Renderer_BeginFrame(leftSurface);
    NeoWPF::Rendering::Renderer_Execute(leftSurface, leftCommands);
    NeoWPF::Rendering::Renderer_Present(leftSurface);

    ASSERT_TRUE(NeoWPF::Rendering::Renderer_Initialize(rightSurface));
    NeoWPF::Rendering::Renderer_Resize(rightSurface, 640, 480);
    NeoWPF::Rendering::CommandList rightCommands;
    rightCommands.DrawRect(20.0, 20.0, 200.0, 160.0,
                           NeoWPF::Rendering::Paint::Solid(0xFF993366u));
    NeoWPF::Rendering::Renderer_BeginFrame(rightSurface);
    NeoWPF::Rendering::Renderer_Execute(rightSurface, rightCommands);
    NeoWPF::Rendering::Renderer_Present(rightSurface);

    NeoWPF::Rendering::Renderer_DestroySurface(rightSurface);
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_IsInitialized());
    EXPECT_FALSE(NeoWPF::Rendering::Renderer_Initialize(rightSurface));
    const NeoWPF::Rendering::RenderSurface recreatedRightSurface{rightWindow.Handle()};
    ASSERT_TRUE(NeoWPF::Rendering::Renderer_Initialize(recreatedRightSurface));
    NeoWPF::Rendering::Renderer_Resize(recreatedRightSurface, 640, 480);
    NeoWPF::Rendering::Renderer_BeginFrame(rightSurface);
    NeoWPF::Rendering::Renderer_Execute(rightSurface, rightCommands);
    NeoWPF::Rendering::Renderer_BeginFrame(recreatedRightSurface);
    NeoWPF::Rendering::Renderer_Execute(recreatedRightSurface, rightCommands);
    NeoWPF::Rendering::Renderer_Present(recreatedRightSurface);

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

    const auto inheritedOpaque = SelectSurfaceCompositeAlpha(
        false, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
    ASSERT_TRUE(inheritedOpaque.has_value());
    EXPECT_EQ(*inheritedOpaque, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
    EXPECT_FALSE(SelectSurfaceCompositeAlpha(
        true, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR).has_value());
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

TEST(VulkanRenderingTests, RejectsMissingSurfaceDuringDeviceInitialization)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;

    EXPECT_FALSE(rendering.InitializeDevice(NeoWPF::Rendering::RenderSurface{}));
    EXPECT_FALSE(rendering.CreateSurface(NeoWPF::Rendering::RenderSurface{}));
    rendering.DestroySurface(NeoWPF::Rendering::RenderSurface{});
    EXPECT_FALSE(rendering.IsInitialized());
}

TEST(VulkanRenderingTests, ShutdownIsIdempotentBeforeInitialization)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;

    rendering.ShutdownDevice();
    rendering.ShutdownDevice();

    EXPECT_FALSE(rendering.IsInitialized());
}

TEST(VulkanRenderingTests, FrameAndTextureOperationsAreSafeBeforeInitialization)
{
    NeoWPF::Rendering::Vulkan::VulkanRendering rendering;
    NeoWPF::Rendering::CommandList commands;

    const NeoWPF::Rendering::RenderSurface surface{};
    rendering.BeginFrame(surface);
    rendering.Execute(surface, commands);
    rendering.Present(surface);
    rendering.DestroyTexture(1);
    rendering.ClearTextures();

    EXPECT_EQ(rendering.CreateTexture(1, 1, nullptr), 0u);
    EXPECT_FALSE(rendering.IsInitialized());
}

#endif

}
