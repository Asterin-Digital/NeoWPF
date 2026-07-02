#pragma once

#include <cstdint>
#include <string>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>

namespace NeoWPF::UI {

class TextBoxStyle;  
void RegisterTextBoxStyles();

class TextBoxTemplate; 

class TextBox : public FrameworkElement {
public:
    static bool s_textBoxDebug;

    TextBox();
    ~TextBox();

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }
    int GetFontSize() const noexcept { return m_fontSize; }

    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }
    int GetPadding() const noexcept { return m_padding; }

    void SetBorderColor(uint32_t color) noexcept { m_borderColor = color; InvalidateVisual(); }
    uint32_t GetBorderColor() const noexcept { return m_borderColor; }

    void SetCaretColor(uint32_t color) noexcept { m_caretColor = color; InvalidateVisual(); }
    uint32_t GetCaretColor() const noexcept { return m_caretColor; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return m_isEnabled; }

    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) override;
    void OnGotFocus() override;
    void OnLostFocus() override;

private:
    std::wstring m_text;
    std::size_t m_caretPosition = 0;
    uint32_t m_textColor = 0xFF000000u;
    uint32_t m_backgroundColor = 0xFFFFFFFFu;
    uint32_t m_borderColor = 0xFF666666u;
    uint32_t m_caretColor = 0xFF000000u;
    int m_fontSize = 16;
    int m_padding = 4;
    bool m_isEnabled = true;

    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    bool m_textDirty = true;
    bool m_isFocused = false;

    void EnsureTextTexture();
    void InsertCharacter(wchar_t character);
    void RemoveCharacter(bool backspace);
    void UpdateCaretFromMouse(int x);
    int GetCaretX() const;
    int GetCaretHeight() const;
};

class TextBoxTemplate : public Templates::ControlTemplate {
public:
    TextBoxTemplate();
};

}
