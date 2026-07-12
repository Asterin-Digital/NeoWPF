#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <NeoWPF/UI/Core/Transform/Transform.h>

namespace NeoWPF::UI {

namespace {

float GetAbsoluteRenderTransformRotation(const UIElement* element)
{
    using namespace NeoWPF::UI::Core;
    Matrix4 matrix = Matrix4::Identity();
    const UIElement* current = element;
    while (current) {
        if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
            matrix = matrix * Matrix4::Translation(
                static_cast<float>(fe->GetLayoutOffsetX() + fe->GetRenderOffsetX()),
                static_cast<float>(fe->GetLayoutOffsetY() + fe->GetRenderOffsetY()));
        }

        if (auto transform = current->GetRenderTransform()) {
            float width = 0.0f;
            float height = 0.0f;
            if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
                width = static_cast<float>((std::max)(fe->GetFinalWidth(), fe->GetDesiredWidth()));
                height = static_cast<float>((std::max)(fe->GetFinalHeight(), fe->GetDesiredHeight()));
            }

            const auto origin = current->GetRenderTransformOrigin();
            const float originX = width * origin.X;
            const float originY = height * origin.Y;

            if (originX != 0.0f || originY != 0.0f) {
                matrix = matrix * Matrix4::Translation(originX, originY);
                matrix = matrix * transform->GetMatrix();
                matrix = matrix * Matrix4::Translation(-originX, -originY);
            } else {
                matrix = matrix * transform->GetMatrix();
            }
        }

        current = dynamic_cast<const UIElement*>(current->GetParent());
    }

    return std::atan2(matrix.m[1][0], matrix.m[0][0]);
}

}

TextBlock::TextBlock()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

TextBlock::TextBlock(const std::wstring& text) : TextBlock()
{
    SetText(text);
}

TextBlock::~TextBlock()
{
    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }
}

void TextBlock::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }
    m_text = text;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBlock::Measure(int availableWidth, int availableHeight)
{
    if (m_text.empty()) {
        SetDesiredSize(0, 0);
        return;
    }

    const int widthConstraint = availableWidth > 0 ? availableWidth : 0;
    const int heightConstraint = availableHeight > 0 ? availableHeight : 0;
    auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, widthConstraint, heightConstraint);
    SetDesiredSize(metrics.width, metrics.height);
}

void TextBlock::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (m_textDirty || finalWidth != m_lastArrangeWidth || finalHeight != m_lastArrangeHeight) {
        EnsureTextTexture(finalWidth, finalHeight);
        m_lastArrangeWidth = finalWidth;
        m_lastArrangeHeight = finalHeight;
    }
}

void TextBlock::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_text.empty() || m_textWidth == 0 || m_textHeight == 0 || m_textureId == 0) {
        return;
    }

    const auto clip = GetClipRect();
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    float drawX = static_cast<float>(absX);
    float drawY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(m_textWidth);
    float drawHeight = static_cast<float>(m_textHeight);
    ApplyRenderTransform(drawX, drawY, drawWidth, drawHeight);
    float rotationRadians = GetAbsoluteRenderTransformRotation(this);

    commands.DrawTexturedQuad(
        drawX,
        drawY,
        drawWidth,
        drawHeight,
        0.0f, 0.0f,
        1.0f, 1.0f,
        m_textureId,
        GetOpacity(),
        clip,
        rotationRadians);
}

void TextBlock::EnsureTextTexture(int availableWidth, int availableHeight)
{
    if (!m_textDirty && availableWidth == m_lastArrangeWidth && availableHeight == m_lastArrangeHeight) {
        return;
    }

    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }

    if (m_text.empty()) {
        m_textWidth = 0;
        m_textHeight = 0;
        m_textDirty = false;
        return;
    }

    int width = 0;
    int height = 0;
    const int widthConstraint = std::max(1, availableWidth);
    const int heightConstraint = std::max(1, availableHeight);
    auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(m_text, m_fontSize, widthConstraint, heightConstraint, width, height);
    if (bitmap.empty() || width == 0 || height == 0) {
        NeoWPF::Diagnostics::LogError("TextBlock: failed to render text bitmap");
        m_textWidth = 0;
        m_textHeight = 0;
        m_textDirty = false;
        return;
    }

    m_textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), bitmap.data());
    m_textWidth = width;
    m_textHeight = height;
    m_textDirty = false;
}

}
