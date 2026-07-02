#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace NeoWPF::UI {

TextBox::TextBox()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

TextBox::~TextBox()
{
    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }
}

void TextBox::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }

    m_text = text;
    m_caretPosition = std::min(m_caretPosition, m_text.size());
    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const int minWidth = 120;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (m_padding * 2);

    if (m_text.empty()) {
        SetDesiredSize(minWidth, minHeight);
        return;
    }

    auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, 0, 0);
    const int desiredWidth = std::max(metrics.width + (m_padding * 2), minWidth);
    const int desiredHeight = std::max(metrics.height + (m_padding * 2), minHeight);
    SetDesiredSize(desiredWidth, desiredHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (m_textDirty) {
        EnsureTextTexture();
    }
}

void TextBox::OnGotFocus()
{
    m_isFocused = true;
    InvalidateVisual();
}

void TextBox::OnLostFocus()
{
    m_isFocused = false;
    InvalidateVisual();
}

void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY),
        static_cast<float>(GetFinalWidth()),
        static_cast<float>(GetFinalHeight()),
        m_backgroundColor);

    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY),
        static_cast<float>(GetFinalWidth()),
        1.0f,
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY + GetFinalHeight() - 1),
        static_cast<float>(GetFinalWidth()),
        1.0f,
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY),
        1.0f,
        static_cast<float>(GetFinalHeight()),
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(absX + GetFinalWidth() - 1),
        static_cast<float>(absY),
        1.0f,
        static_cast<float>(GetFinalHeight()),
        m_borderColor);

    if (m_textureId != 0 && m_textWidth > 0 && m_textHeight > 0) {
        commands.DrawTexturedQuad(
            static_cast<float>(absX + m_padding),
            static_cast<float>(absY + m_padding),
            static_cast<float>(m_textWidth),
            static_cast<float>(m_textHeight),
            0.0f, 0.0f,
            1.0f, 1.0f,
            m_textureId);
    }

    if (m_isFocused) {
        const int caretX = GetCaretX();
        const int caretHeight = std::max(1, GetCaretHeight());
        commands.DrawRect(
            static_cast<float>(absX + m_padding + caretX),
            static_cast<float>(absY + m_padding),
            1.0f,
            static_cast<float>(caretHeight),
            m_caretColor);
    }
}

void TextBox::EnsureTextTexture()
{
    if (!m_textDirty) {
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
    auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(m_text, m_fontSize, width, height);
    if (bitmap.empty() || width == 0 || height == 0) {
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
