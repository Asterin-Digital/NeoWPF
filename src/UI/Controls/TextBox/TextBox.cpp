#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>

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
    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    if (m_text.empty()) {
        SetDesiredSize(120, m_fontSize + (m_padding * 2));
        return;
    }

    auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, 0, 0);
    const int desiredWidth = (metrics.width + (m_padding * 2) > 120) ? (metrics.width + (m_padding * 2)) : 120;
    SetDesiredSize(desiredWidth, metrics.height + (m_padding * 2));
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (m_textDirty) {
        EnsureTextTexture();
    }
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
        0xFF666666u);
    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY + GetFinalHeight() - 1),
        static_cast<float>(GetFinalWidth()),
        1.0f,
        0xFF666666u);
    commands.DrawRect(
        static_cast<float>(absX),
        static_cast<float>(absY),
        1.0f,
        static_cast<float>(GetFinalHeight()),
        0xFF666666u);
    commands.DrawRect(
        static_cast<float>(absX + GetFinalWidth() - 1),
        static_cast<float>(absY),
        1.0f,
        static_cast<float>(GetFinalHeight()),
        0xFF666666u);

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
