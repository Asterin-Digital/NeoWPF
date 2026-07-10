#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

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

    int absX, absY;
    GetAbsolutePosition(absX, absY);

    const auto clip = GetClipRect();
    commands.DrawTexturedQuad(
        static_cast<float>(absX),
        static_cast<float>(absY),
        static_cast<float>(m_textWidth),
        static_cast<float>(m_textHeight),
        0.0f, 0.0f,
        1.0f, 1.0f,
        m_textureId,
        GetOpacity(),
        clip);
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
