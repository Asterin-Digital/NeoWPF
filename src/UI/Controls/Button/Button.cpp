#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Button/ButtonTemplate.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <memory>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

Button::Button()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ButtonTemplate>());
    ApplyTemplate();
}

Button::Button(const std::wstring& text)
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ButtonTemplate>());
    ApplyTemplate();
    SetText(text);
}

Button::~Button()
{
    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }
}

void Button::SetText(const std::wstring& text)
{
    m_text = text;
    m_textDirty = true;
    InvalidateVisual();
}

void Button::Measure(int availableWidth, int availableHeight)
{
    if (m_text.empty()) {
        SetDesiredSize(m_padding * 2, m_padding * 2);
        return;
    }

    auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, 0, 0);
    int measuredWidth = metrics.width + (m_padding * 2);
    int measuredHeight = metrics.height + (m_padding * 2);
    SetDesiredSize(measuredWidth, measuredHeight);
}

void Button::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (m_textDirty) {
        EnsureTextTexture();
    }
}

void Button::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    auto* background = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        background->BuildRenderCommands(commands);
    }
    else {
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
            0xFF333333u);
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY + GetFinalHeight() - 1),
            static_cast<float>(GetFinalWidth()),
            1.0f,
            0xFF333333u);
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            0xFF333333u);
        commands.DrawRect(
            static_cast<float>(absX + GetFinalWidth() - 1),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            0xFF333333u);
    }

    auto* contentHost = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const int inset = std::max(0, m_padding);
        contentHost->SetRect(absX + inset, absY + inset, std::max(0, GetFinalWidth() - (inset * 2)), std::max(0, GetFinalHeight() - (inset * 2)));
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);
        contentHost->BuildRenderCommands(commands);
    }


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

void Button::EnsureTextTexture()
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

void Button::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (m_isEnabled) {
        RaiseClick();
    }
}

void Button::RaiseClick()
{
    Click.Invoke();
}

}
