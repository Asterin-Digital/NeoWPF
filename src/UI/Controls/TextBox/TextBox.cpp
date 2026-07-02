#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Diagnostics/Logger.h>
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

bool NeoWPF::UI::TextBox::s_textBoxDebug = true; // Please do not turn this on unless ur debugging something

namespace NeoWPF::UI {

TextBox::TextBox()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Stretch);
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

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] SetText: oldLength=%zu, newLength=%zu", m_text.size(), text.size());
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

    int desiredWidth = availableWidth > 0 ? availableWidth : minWidth;
    int desiredHeight = minHeight;
    if (!m_text.empty()) {
        const auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, 0, 0);
        desiredHeight = std::max(metrics.height + (m_padding * 2), minHeight);
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());
    }

    SetDesiredSize(desiredWidth, desiredHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Arrange: final=%dx%d, textDirty=%d, tex=%dx%d", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_textWidth, m_textHeight);
    }
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
        const int visibleTextWidth = std::max(0, GetFinalWidth() - (m_padding * 2));
        const int drawWidth = std::min(m_textWidth, visibleTextWidth);
        const float uvRight = drawWidth > 0 ? static_cast<float>(drawWidth) / static_cast<float>(m_textWidth) : 1.0f;

        commands.DrawTexturedQuad(
            static_cast<float>(absX + m_padding),
            static_cast<float>(absY + m_padding),
            static_cast<float>(drawWidth),
            static_cast<float>(m_textHeight),
            0.0f, 0.0f,
            uvRight, 1.0f,
            m_textureId);
    }

    if (m_isFocused) {
        const int visibleTextWidth = std::max(0, GetFinalWidth() - (m_padding * 2));
        int caretX = GetCaretX();
        caretX = std::min(caretX, visibleTextWidth);
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

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] EnsureTextTexture: textLength=%zu, oldTexture=%u", m_text.size(), m_textureId);
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
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] RenderTextBitmap result: width=%d height=%d, bitmapSize=%zu", width, height, bitmap.size());
    }
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
