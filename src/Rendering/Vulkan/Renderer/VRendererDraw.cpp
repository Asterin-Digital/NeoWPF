#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Rendering/Geometry/Clipping.h>
#include <NeoWPF/Rendering/Geometry/ArcTessellation.h>
#include <NeoWPF/Rendering/Geometry/PrimitiveTessellation.h>
#include <NeoWPF/Rendering/Geometry/DrawBatcher.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {

void VulkanRendering::Execute(const NeoWPF::Rendering::RenderSurface& surface,
                              const NeoWPF::Rendering::CommandList& commands)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    auto* context = FindWindowContext(surface.nativeWindowHandle);
    if (!context || !context->frameActive) {
        static bool loggedInactiveContext = false;
        if (!loggedInactiveContext) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Debug,
                "[Vulkan] Execute skipped: window context unavailable or frame inactive");
            loggedInactiveContext = true;
        }
        return;
    }

    std::size_t rectCount = 0;
    std::size_t roundedRectCount = 0;
    std::size_t arcCount = 0;
    std::size_t texturedQuadCount = 0;
    for (const auto& command : commands.commands) {
        switch (command.GetKind()) {
        case NeoWPF::Rendering::CommandList::CommandKind::Rect:
            ++rectCount;
            break;
        case NeoWPF::Rendering::CommandList::CommandKind::RoundedRect:
            ++roundedRectCount;
            break;
        case NeoWPF::Rendering::CommandList::CommandKind::Arc:
            ++arcCount;
            break;
        case NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad:
            ++texturedQuadCount;
            break;
        }
    }
    static std::size_t lastCommandCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastRoundedRectCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastArcCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastTexturedQuadCount = std::numeric_limits<std::size_t>::max();
    if (commands.commands.size() != lastCommandCount ||
        roundedRectCount != lastRoundedRectCount ||
        arcCount != lastArcCount ||
        texturedQuadCount != lastTexturedQuadCount) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[Vulkan] Command shape changed commands=%zu rects=%zu roundedRects=%zu arcs=%zu texturedQuads=%zu",
            commands.commands.size(), rectCount, roundedRectCount, arcCount, texturedQuadCount);
        lastCommandCount = commands.commands.size();
        lastRoundedRectCount = roundedRectCount;
        lastArcCount = arcCount;
        lastTexturedQuadCount = texturedQuadCount;
    }

    std::uint32_t backgroundColor = 0xFFFFFFFFu;
    if (commands.hasClearColor) {
        backgroundColor = commands.clearColor;
    } else {
        std::shared_ptr<NeoWPF::UI::Brush> resourceBrush;
        if (NeoWPF::Core::ApplicationResources::Resources::TryGetBrush(
                "Window.BackgroundBrush", resourceBrush)) {
            if (const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(resourceBrush)) {
                backgroundColor = solidBrush->GetArgb();
            }
        }
    }
    const auto clearColor = ToSurfaceClearColor(backgroundColor, context->transparent);
    const VkClearAttachment clearAttachment{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        {{clearColor[0], clearColor[1], clearColor[2], clearColor[3]}}};
    const VkClearRect clearRect{{{0, 0}, context->swapchainExtent}, 0, 1};
    vkCmdClearAttachments(context->commandBuffers[context->currentImage], 1, &clearAttachment, 1, &clearRect);

    struct Vertex { float x, y, r, g, b, a; };
    struct TexturedVertex { float x, y, u, v, r, g, b, a; };
    const VkDeviceSize offset = 0;
    const float viewport[] = {static_cast<float>(context->swapchainExtent.width), static_cast<float>(context->swapchainExtent.height)};
    const VkViewport vkViewport{0.0f, static_cast<float>(context->swapchainExtent.height),
                                static_cast<float>(context->swapchainExtent.width),
                                -static_cast<float>(context->swapchainExtent.height), 0.0f, 1.0f};
    const VkRect2D scissor{{0, 0}, context->swapchainExtent};
    const VkCommandBuffer commandBuffer = context->commandBuffers[context->currentImage];
    struct PendingBatch {
        DrawBatchKind kind;
        std::size_t firstVertex;
        std::size_t vertexCount;
        std::uint32_t textureId;
        VkDescriptorSet descriptor;
    };
    std::vector<Vertex> solidVertices;
    std::vector<TexturedVertex> texturedVertices;
    std::vector<GeometryVertex> geometry;
    std::vector<PendingBatch> pendingBatches;
    const auto batches = BuildDrawBatches(commands);
    pendingBatches.reserve(batches.size());

    for (const auto& batch : batches) {
        const std::size_t firstCommand = batch.firstCommand;
        const std::size_t endCommand = firstCommand + batch.commandCount;
        if (batch.kind == DrawBatchKind::Solid) {
            const std::size_t firstVertex = solidVertices.size();
            for (std::size_t commandIndex = firstCommand; commandIndex < endCommand; ++commandIndex) {
                const auto& command = commands.commands[commandIndex];
                if (command.GetKind() == CommandList::CommandKind::Arc) {
                    const auto& arc = command.GetArc();
                    const float alpha = ((arc.paint.color >> 24) & 0xFF) / 255.0f * arc.opacity;
                    const float red = ((arc.paint.color >> 16) & 0xFF) / 255.0f;
                    const float green = ((arc.paint.color >> 8) & 0xFF) / 255.0f;
                    const float blue = (arc.paint.color & 0xFF) / 255.0f;
                    TessellateAnnularArc(geometry,
                        arc.centerX, arc.centerY, arc.radius, arc.thickness,
                        arc.startAngleRadians, arc.sweepAngleRadians, arc.clip);
                    for (const auto& vertex : geometry) {
                        solidVertices.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                                 red, green, blue, alpha});
                    }
                    continue;
                }

                const bool rounded = command.GetKind() == CommandList::CommandKind::RoundedRect;
                const std::uint32_t color = rounded
                    ? command.GetRoundedRect().paint.color
                    : command.GetRect().paint.color;
                const float opacity = rounded
                    ? command.GetRoundedRect().opacity
                    : command.GetRect().opacity;
                const float alpha = ((color >> 24) & 0xFF) / 255.0f * opacity;
                const float red = ((color >> 16) & 0xFF) / 255.0f;
                const float green = ((color >> 8) & 0xFF) / 255.0f;
                const float blue = (color & 0xFF) / 255.0f;
                if (rounded) {
                    const auto& rect = command.GetRoundedRect();
                    TessellateRoundedRect(geometry,
                        rect.x, rect.y, rect.w, rect.h, rect.topLeft, rect.topRight,
                        rect.bottomRight, rect.bottomLeft, rect.clip,
                        rect.rotationRadians, rect.pivotX, rect.pivotY);
                } else {
                    const auto& rect = command.GetRect();
                    TessellateRect(geometry,
                        rect.x, rect.y, rect.w, rect.h, rect.clip,
                        rect.rotationRadians, rect.pivotX, rect.pivotY);
                }
                for (const auto& vertex : geometry) {
                    solidVertices.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                             red, green, blue, alpha});
                }
            }
            pendingBatches.push_back({DrawBatchKind::Solid, firstVertex,
                                      solidVertices.size() - firstVertex, 0, VK_NULL_HANDLE});
            continue;
        }

        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        {
            std::lock_guard<std::mutex> lock(m_textureMutex);
            const auto texture = m_textures.find(batch.textureId);
            if (texture != m_textures.end()) {
                descriptor = texture->second.descriptor;
            }
        }
        const std::size_t firstVertex = texturedVertices.size();
        if (descriptor != VK_NULL_HANDLE) {
            for (std::size_t commandIndex = firstCommand; commandIndex < endCommand; ++commandIndex) {
                const auto& quad = commands.commands[commandIndex].GetTexturedQuad();
                TessellateRect(geometry,
                    quad.x, quad.y, quad.w, quad.h, quad.clip,
                    quad.rotationRadians, quad.pivotX, quad.pivotY,
                    quad.u0, quad.v0, quad.u1, quad.v1);
                const float alpha = ((quad.tint.color >> 24) & 0xFF) / 255.0f * quad.opacity;
                const float red = ((quad.tint.color >> 16) & 0xFF) / 255.0f;
                const float green = ((quad.tint.color >> 8) & 0xFF) / 255.0f;
                const float blue = (quad.tint.color & 0xFF) / 255.0f;
                for (const auto& vertex : geometry) {
                    texturedVertices.push_back({static_cast<float>(vertex.x), static_cast<float>(vertex.y),
                                                vertex.u, vertex.v, red, green, blue, alpha});
                }
            }
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Debug,
                "[Vulkan] Skipping textured batch with missing texture id=%u", batch.textureId);
        }
        pendingBatches.push_back({DrawBatchKind::Textured, firstVertex,
                                  texturedVertices.size() - firstVertex,
                                  batch.textureId, descriptor});
    }

    const VkDeviceSize solidByteCount = solidVertices.size() * sizeof(Vertex);
    const VkDeviceSize texturedByteCount = texturedVertices.size() * sizeof(TexturedVertex);
    const bool solidBufferReady = solidVertices.empty() || EnsureVertexBuffer(
        context->solidBuffer, solidByteCount);
    const bool texturedBufferReady = texturedVertices.empty() || EnsureVertexBuffer(
        context->texturedBuffer, texturedByteCount);
    bool solidUploadSucceeded = solidVertices.empty();
    bool texturedUploadSucceeded = texturedVertices.empty();
    if (!solidBufferReady) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[Vulkan] Could not grow the solid vertex buffer");
    } else if (!solidVertices.empty() && context->solidPipeline != VK_NULL_HANDLE &&
               context->solidBuffer.buffer != VK_NULL_HANDLE) {
        void* mapped = nullptr;
        if (vkMapMemory(m_device, context->solidBuffer.memory, 0, solidByteCount, 0, &mapped) == VK_SUCCESS) {
            std::memcpy(mapped, solidVertices.data(), static_cast<std::size_t>(solidByteCount));
            vkUnmapMemory(m_device, context->solidBuffer.memory);
            solidUploadSucceeded = true;
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Debug,
                "[Vulkan] Failed to map the solid vertex stream");
        }
    }
    if (!texturedBufferReady) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[Vulkan] Could not grow the textured vertex buffer");
    } else if (!texturedVertices.empty() && context->texturedPipeline != VK_NULL_HANDLE &&
               context->texturedBuffer.buffer != VK_NULL_HANDLE) {
        void* mapped = nullptr;
        if (vkMapMemory(m_device, context->texturedBuffer.memory, 0, texturedByteCount, 0, &mapped) == VK_SUCCESS) {
            std::memcpy(mapped, texturedVertices.data(), static_cast<std::size_t>(texturedByteCount));
            vkUnmapMemory(m_device, context->texturedBuffer.memory);
            texturedUploadSucceeded = true;
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Debug,
                "[Vulkan] Failed to map the textured vertex stream");
        }
    }

    for (const auto& batch : pendingBatches) {
        if (batch.vertexCount == 0) {
            continue;
        }
        if (batch.kind == DrawBatchKind::Solid) {
            if (!solidBufferReady || !solidUploadSucceeded || context->solidPipeline == VK_NULL_HANDLE ||
                context->solidBuffer.buffer == VK_NULL_HANDLE) {
                continue;
            }
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->solidPipeline);
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &context->solidBuffer.buffer, &offset);
            vkCmdPushConstants(commandBuffer, context->solidPipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(viewport), viewport);
        } else {
            if (!texturedBufferReady || !texturedUploadSucceeded || context->texturedPipeline == VK_NULL_HANDLE ||
                context->texturedBuffer.buffer == VK_NULL_HANDLE || batch.descriptor == VK_NULL_HANDLE) {
                continue;
            }
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->texturedPipeline);
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &context->texturedBuffer.buffer, &offset);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    context->texturedPipelineLayout, 0, 1, &batch.descriptor, 0, nullptr);
            vkCmdPushConstants(commandBuffer, context->texturedPipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(viewport), viewport);
        }
        vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdDraw(commandBuffer, static_cast<std::uint32_t>(batch.vertexCount), 1,
                  static_cast<std::uint32_t>(batch.firstVertex), 0);
    }
}

}
