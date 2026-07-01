#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <cmath>
#include <limits>

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

void TextBox::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (!m_isEnabled) {
        return;
    }

    UpdateCaretFromMouse(x);
    m_isFocused = true;
    InvalidateVisual();
}

void TextBox::OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args)
{
    if (!args || !m_isEnabled || !args->IsKeyDown()) {
        return;
    }

    const int keyCode = args->GetKeyCode();
    const wchar_t charCode = args->GetCharCode();
    bool changed = false;

    if (charCode != 0 && charCode != L'\r' && charCode != L'\n' && charCode != L'\t' && charCode != 8) {
        InsertCharacter(charCode);
        changed = true;
    }
    else {
        switch (keyCode) {
        case VK_BACK:
            RemoveCharacter(true);
            changed = true;
            break;
        case VK_DELETE:
            RemoveCharacter(false);
            changed = true;
            break;
        case VK_LEFT:
            if (m_caretPosition > 0) {
                --m_caretPosition;
                changed = true;
            }
            break;
        case VK_RIGHT:
            if (m_caretPosition < m_text.size()) {
                ++m_caretPosition;
                changed = true;
            }
            break;
        case VK_HOME:
            m_caretPosition = 0;
            changed = true;
            break;
        case VK_END:
            m_caretPosition = m_text.size();
            changed = true;
            break;
        case VK_RETURN:
        case VK_TAB:
            args->SetHandled(true);
            return;
        default:
            break;
        }
    }

    if (changed) {
        args->SetHandled(true);
        InvalidateVisual();
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
        const int caretHeight = std::max(1, m_textHeight > 0 ? m_textHeight : m_fontSize);
        commands.DrawRect(
            static_cast<float>(absX + m_padding + caretX),
            static_cast<float>(absY + m_padding),
            1.0f,
            static_cast<float>(caretHeight),
            m_caretColor);
    }
}

void TextBox::InsertCharacter(wchar_t character)
{
    m_text.insert(m_caretPosition, 1, character);
    ++m_caretPosition;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::RemoveCharacter(bool backspace)
{
    if (backspace) {
        if (m_caretPosition == 0) {
            return;
        }
        m_text.erase(m_caretPosition - 1, 1);
        --m_caretPosition;
    }
    else if (m_caretPosition < m_text.size()) {
        m_text.erase(m_caretPosition, 1);
    }

    m_textDirty = true;
    InvalidateVisual();
}

void TextBox::UpdateCaretFromMouse(int x)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const int localX = x - absX - m_padding;
    if (localX <= 0) {
        m_caretPosition = 0;
        return;
    }

    std::size_t bestPosition = 0;
    int bestDistance = std::numeric_limits<int>::max();
    for (std::size_t i = 0; i <= m_text.size(); ++i) {
        const std::wstring prefix = m_text.substr(0, i);
        const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
        const int distance = std::abs(localX - metrics.width);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestPosition = i;
        }
    }

    m_caretPosition = bestPosition;
}

int TextBox::GetCaretX() const
{
    if (m_caretPosition == 0) {
        return 0;
    }

    const std::wstring prefix = m_text.substr(0, m_caretPosition);
    const auto metrics = NeoWPF::Rendering::Text::MeasureText(prefix, m_fontSize, 0, 0);
    return metrics.width;
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
