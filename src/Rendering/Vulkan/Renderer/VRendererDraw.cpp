#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Rendering/Clipping.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/Vulkan/VulkanRendering.h>
#include <NeoWPF/Rendering/Vulkan/SurfaceAlpha.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace NeoWPF::Rendering::Vulkan {

void VulkanRendering::Execute(const NeoWPF::Rendering::CommandList& commands)
{
    std::lock_guard<std::recursive_mutex> renderLock(m_renderMutex);
    auto* context = CurrentWindowContext();
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
    std::size_t texturedQuadCount = 0;
    for (const auto& command : commands.commands) {
        switch (command.kind) {
        case NeoWPF::Rendering::CommandList::CommandKind::Rect:
            ++rectCount;
            break;
        case NeoWPF::Rendering::CommandList::CommandKind::RoundedRect:
            ++roundedRectCount;
            break;
        case NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad:
            ++texturedQuadCount;
            break;
        }
    }
    static std::size_t lastCommandCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastRoundedRectCount = std::numeric_limits<std::size_t>::max();
    static std::size_t lastTexturedQuadCount = std::numeric_limits<std::size_t>::max();
    if (commands.commands.size() != lastCommandCount ||
        roundedRectCount != lastRoundedRectCount ||
        texturedQuadCount != lastTexturedQuadCount) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
            NeoWPF::Diagnostics::Level::Debug,
            "[Vulkan] Command shape changed commands=%zu rects=%zu roundedRects=%zu texturedQuads=%zu",
            commands.commands.size(), rectCount, roundedRectCount, texturedQuadCount);
        lastCommandCount = commands.commands.size();
        lastRoundedRectCount = roundedRectCount;
        lastTexturedQuadCount = texturedQuadCount;
    }

    std::uint32_t backgroundColor = 0xFFFFFFFFu;
    if (commands.hasClearColor) {
        backgroundColor = commands.clearColor;
    } else {
        NeoWPF::UI::Color resourceColor;
        if (NeoWPF::Core::ApplicationResources::Resources::TryGetColor(
                "Window.BackgroundColor", resourceColor)) {
            backgroundColor = resourceColor.ToArgb();
        }
    }
    const auto clearColor = ToSurfaceClearColor(backgroundColor, context->transparent);
    const VkClearAttachment clearAttachment{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        {{clearColor[0], clearColor[1], clearColor[2], clearColor[3]}}};
    const VkClearRect clearRect{{{0, 0}, context->swapchainExtent}, 0, 1};
    vkCmdClearAttachments(context->commandBuffers[context->currentImage], 1, &clearAttachment, 1, &clearRect);

    if (m_solidPipeline == VK_NULL_HANDLE || m_solidVertexBuffer == VK_NULL_HANDLE) {
        static bool loggedMissingPipeline = false;
        if (!loggedMissingPipeline) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Renderer,
                NeoWPF::Diagnostics::Level::Info,
                "[Vulkan] Execute skipped: solid pipeline or vertex buffer unavailable");
            loggedMissingPipeline = true;
        }
        return;
    }

    struct Vertex { float x, y, r, g, b, a; };
    struct Point { float x, y; };
    std::vector<Vertex> vertices;
    vertices.reserve(commands.commands.size() * 6);
    const auto makePoint = [](double x, double y) {
        return Point{static_cast<float>(x), static_cast<float>(y)};
    };
    const auto appendVertex = [&vertices](double x, double y, float red, float green, float blue, float alpha) {
        vertices.push_back({static_cast<float>(x), static_cast<float>(y), red, green, blue, alpha});
    };
    const auto rotatePoint = [](Point point, Point center, float angle) {
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        const float dx = point.x - center.x;
        const float dy = point.y - center.y;
        return Point{center.x + dx * cosine - dy * sine,
                     center.y + dx * sine + dy * cosine};
    };
    const auto appendRoundedRect = [&](const auto& rect, float red, float green, float blue, float alpha) {
        if (rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const float radiusLimit = std::min(rect.w * 0.5f, rect.h * 0.5f);
        const auto clampRadius = [radiusLimit](float radius) {
            return std::clamp(radius, 0.0f, radiusLimit);
        };
        const float topLeft = clampRadius(rect.topLeft);
        const float topRight = clampRadius(rect.topRight);
        const float bottomRight = clampRadius(rect.bottomRight);
        const float bottomLeft = clampRadius(rect.bottomLeft);
        constexpr float pi = 3.14159265358979323846f;
        std::vector<Point> outline;
        outline.reserve(36);
        const auto addArc = [&outline, &makePoint](Point center, float start, float end, float radius) {
            if (radius <= 0.0f) {
                outline.push_back(center);
                return;
            }
            for (int index = 0; index <= 8; ++index) {
                const float t = static_cast<float>(index) / 8.0f;
                const float angle = start + (end - start) * t;
                outline.push_back(makePoint(center.x + radius * std::cos(angle),
                                            center.y + radius * std::sin(angle)));
            }
        };
        outline.push_back(makePoint(rect.x + topLeft, rect.y));
        outline.push_back(makePoint(rect.x + rect.w - topRight, rect.y));
        addArc(makePoint(rect.x + rect.w - topRight, rect.y + topRight), -pi * 0.5f, 0.0f, topRight);
        addArc(makePoint(rect.x + rect.w - bottomRight, rect.y + rect.h - bottomRight), 0.0f, pi * 0.5f, bottomRight);
        addArc(makePoint(rect.x + bottomLeft, rect.y + rect.h - bottomLeft), pi * 0.5f, pi, bottomLeft);
        addArc(makePoint(rect.x + topLeft, rect.y + topLeft), pi, pi * 1.5f, topLeft);
        const Point center = makePoint(rect.pivotX, rect.pivotY);
        for (std::size_t index = 1; index + 1 < outline.size(); ++index) {
            const Point p0 = rotatePoint(outline[0], center, rect.rotationRadians);
            const Point p1 = rotatePoint(outline[index], center, rect.rotationRadians);
            const Point p2 = rotatePoint(outline[index + 1], center, rect.rotationRadians);
            appendVertex(p0.x, p0.y, red, green, blue, alpha);
            appendVertex(p1.x, p1.y, red, green, blue, alpha);
            appendVertex(p2.x, p2.y, red, green, blue, alpha);
        }
    };
    for (const auto& command : commands.commands) {
        if (command.kind != NeoWPF::Rendering::CommandList::CommandKind::Rect &&
            command.kind != NeoWPF::Rendering::CommandList::CommandKind::RoundedRect) {
            continue;
        }
        const bool rounded = command.kind == NeoWPF::Rendering::CommandList::CommandKind::RoundedRect;
        const double xValue = rounded ? command.roundedRect.x : command.rect.x;
        const double yValue = rounded ? command.roundedRect.y : command.rect.y;
        const double widthValue = rounded ? command.roundedRect.w : command.rect.w;
        const double heightValue = rounded ? command.roundedRect.h : command.rect.h;
        const std::uint32_t color = rounded ? command.roundedRect.color : command.rect.color;
        const float opacity = rounded ? command.roundedRect.opacity : command.rect.opacity;
        const float rotation = rounded ? command.roundedRect.rotationRadians : command.rect.rotationRadians;
        double x = xValue;
        double y = yValue;
        double width = widthValue;
        double height = heightValue;
        if (std::fabs(rotation) <= 1e-6f && NeoWPF::Rendering::ClipRectToBounds(
            xValue, yValue, widthValue, heightValue,
            rounded ? command.roundedRect.clip : command.rect.clip, x, y, width, height) &&
            (width <= 0.0f || height <= 0.0f)) {
            continue;
        }
        if (width <= 0.0f || height <= 0.0f) {
            continue;
        }
        const float alpha = ((color >> 24) & 0xFF) / 255.0f * opacity;
        const float red = ((color >> 16) & 0xFF) / 255.0f;
        const float green = ((color >> 8) & 0xFF) / 255.0f;
        const float blue = (color & 0xFF) / 255.0f;
        if (rounded) {
            auto roundedRect = command.roundedRect;
            roundedRect.x = x;
            roundedRect.y = y;
            roundedRect.w = width;
            roundedRect.h = height;
            appendRoundedRect(roundedRect, red, green, blue, alpha);
            continue;
        }
        if (std::fabs(rotation) > 1e-6f) {
            const Point center = makePoint(command.rect.pivotX, command.rect.pivotY);
            const Point points[] = {makePoint(x, y), makePoint(x + width, y), makePoint(x + width, y + height),
                                    makePoint(x + width, y + height), makePoint(x, y + height), makePoint(x, y)};
            for (const auto& point : points) {
                const Point transformed = rotatePoint(point, center, rotation);
                appendVertex(transformed.x, transformed.y, red, green, blue, alpha);
            }
            continue;
        }
        appendVertex(x, y, red, green, blue, alpha);
        appendVertex(x + width, y, red, green, blue, alpha);
        appendVertex(x + width, y + height, red, green, blue, alpha);
        appendVertex(x + width, y + height, red, green, blue, alpha);
        appendVertex(x, y + height, red, green, blue, alpha);
        appendVertex(x, y, red, green, blue, alpha);
    }
    void* mapped = nullptr;
    const VkDeviceSize offset = 0;
    const float viewport[] = {static_cast<float>(context->swapchainExtent.width), static_cast<float>(context->swapchainExtent.height)};
    const VkViewport vkViewport{0.0f, static_cast<float>(context->swapchainExtent.height),
                                static_cast<float>(context->swapchainExtent.width),
                                -static_cast<float>(context->swapchainExtent.height), 0.0f, 1.0f};
    const VkRect2D scissor{{0, 0}, context->swapchainExtent};
    if (!vertices.empty() && vertices.size() * sizeof(Vertex) <= 1024 * 1024) {
        if (vkMapMemory(m_device, m_solidVertexMemory, 0, vertices.size() * sizeof(Vertex), 0, &mapped) != VK_SUCCESS) {
            NeoWPF::Diagnostics::LogError("[Vulkan] Failed to map solid vertex buffer");
        } else {
            std::memcpy(mapped, vertices.data(), vertices.size() * sizeof(Vertex));
            vkUnmapMemory(m_device, m_solidVertexMemory);
            vkCmdBindPipeline(context->commandBuffers[context->currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_solidPipeline);
            vkCmdBindVertexBuffers(context->commandBuffers[context->currentImage], 0, 1, &m_solidVertexBuffer, &offset);
            vkCmdPushConstants(context->commandBuffers[context->currentImage], m_solidPipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(viewport), viewport);
            vkCmdSetViewport(context->commandBuffers[context->currentImage], 0, 1, &vkViewport);
            vkCmdSetScissor(context->commandBuffers[context->currentImage], 0, 1, &scissor);
            vkCmdDraw(context->commandBuffers[context->currentImage], static_cast<std::uint32_t>(vertices.size()), 1, 0, 0);
        }
    }

    if (m_texturedPipeline == VK_NULL_HANDLE || m_texturedVertexBuffer == VK_NULL_HANDLE) {
        return;
    }
    struct TexturedVertex { float x, y, u, v, r, g, b, a; };
    std::vector<TexturedVertex> texturedVertices;
    std::vector<VkDescriptorSet> descriptors;
    const auto appendTexturedVertex = [&texturedVertices](double x, double y, float u, float v,
                                                           float red, float green, float blue, float alpha) {
        texturedVertices.push_back({static_cast<float>(x), static_cast<float>(y), u, v,
                                    red, green, blue, alpha});
    };
    texturedVertices.reserve(commands.commands.size() * 6);
    descriptors.reserve(commands.commands.size());
    for (const auto& command : commands.commands) {
        if (command.kind != NeoWPF::Rendering::CommandList::CommandKind::TexturedQuad) {
            continue;
        }
        const auto& quad = command.texturedQuad;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        {
            std::lock_guard<std::mutex> lock(m_textureMutex);
            const auto texture = m_textures.find(quad.textureId);
            if (texture != m_textures.end()) {
                descriptor = texture->second.descriptor;
            }
        }
        if (descriptor == VK_NULL_HANDLE) {
            NeoWPF::Diagnostics::LogWarn("[Vulkan] Skipping textured quad with missing texture id=%u", quad.textureId);
            continue;
        }
        double x = quad.x;
        double y = quad.y;
        double width = quad.w;
        double height = quad.h;
        float u0 = quad.u0;
        float v0 = quad.v0;
        float u1 = quad.u1;
        float v1 = quad.v1;
        if (std::fabs(quad.rotationRadians) <= 1e-6f && NeoWPF::Rendering::ClipRectToBounds(
            quad.x, quad.y, quad.w, quad.h, quad.clip, x, y, width, height)) {
            if (width <= 0.0f || height <= 0.0f || quad.w == 0.0f || quad.h == 0.0f) {
                continue;
            }
            const float uSpan = u1 - u0;
            const float vSpan = v1 - v0;
            u0 += ((x - quad.x) / quad.w) * uSpan;
            v0 += ((y - quad.y) / quad.h) * vSpan;
            u1 = u0 + (width / quad.w) * uSpan;
            v1 = v0 + (height / quad.h) * vSpan;
        }
        if (width <= 0.0f || height <= 0.0f) {
            continue;
        }
        const float alpha = ((quad.tintColor >> 24) & 0xFF) / 255.0f * quad.opacity;
        const float red = ((quad.tintColor >> 16) & 0xFF) / 255.0f;
        const float green = ((quad.tintColor >> 8) & 0xFF) / 255.0f;
        const float blue = (quad.tintColor & 0xFF) / 255.0f;
        const Point center = makePoint(quad.pivotX, quad.pivotY);
        const Point points[] = {makePoint(x, y), makePoint(x + width, y), makePoint(x + width, y + height),
                    makePoint(x + width, y + height), makePoint(x, y + height), makePoint(x, y)};
        const float uvs[] = {u0, v0, u1, v0, u1, v1, u1, v1, u0, v1, u0, v0};
        for (std::size_t index = 0; index < 6; ++index) {
            const Point transformed = rotatePoint(points[index], center, quad.rotationRadians);
            appendTexturedVertex(transformed.x, transformed.y, uvs[index * 2], uvs[index * 2 + 1],
                                 red, green, blue, alpha);
        }
        descriptors.push_back(descriptor);
    }
    if (texturedVertices.empty() || texturedVertices.size() * sizeof(TexturedVertex) > 1024 * 1024) {
        return;
    }
    if (vkMapMemory(m_device, m_texturedVertexMemory, 0,
                    texturedVertices.size() * sizeof(TexturedVertex), 0, &mapped) != VK_SUCCESS) {
        NeoWPF::Diagnostics::LogError("[Vulkan] Failed to map textured vertex buffer");
        return;
    }
    std::memcpy(mapped, texturedVertices.data(), texturedVertices.size() * sizeof(TexturedVertex));
    vkUnmapMemory(m_device, m_texturedVertexMemory);
    vkCmdBindPipeline(context->commandBuffers[context->currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_texturedPipeline);
    vkCmdBindVertexBuffers(context->commandBuffers[context->currentImage], 0, 1, &m_texturedVertexBuffer, &offset);
    vkCmdBindDescriptorSets(context->commandBuffers[context->currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_texturedPipelineLayout, 0, 1, &descriptors[0], 0, nullptr);
    vkCmdPushConstants(context->commandBuffers[context->currentImage], m_texturedPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(viewport), viewport);
    for (std::size_t index = 0; index < descriptors.size(); ++index) {
        vkCmdBindDescriptorSets(context->commandBuffers[context->currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_texturedPipelineLayout, 0, 1, &descriptors[index], 0, nullptr);
        vkCmdDraw(context->commandBuffers[context->currentImage], 6, 1, static_cast<std::uint32_t>(index * 6), 0);
    }
}

}
