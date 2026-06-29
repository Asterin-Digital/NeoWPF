#pragma once

#include <string>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/Core/Events/Event.h>

namespace NeoWPF::Input::Core {
    class MouseEventArgs;
}

namespace NeoWPF::UI {

class Button : public FrameworkElement {
public:
    Button();
    ~Button();

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }
    int GetFontSize() const noexcept { return m_fontSize; }

    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }
    int GetPadding() const noexcept { return m_padding; }

    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return m_isEnabled; }

    // Event handlers
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    // Click event - public for direct subscription with +=
    NeoWPF::Core::Event<> Click;

private:
    std::wstring m_text;
    uint32_t m_textColor = 0xFF000000u;
    uint32_t m_backgroundColor = 0xFFC0C0C0u;
    int m_fontSize = 16;

private:
    int m_padding = 8;
    bool m_isEnabled = true;
    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    bool m_textDirty = true;

    void EnsureTextTexture();
    void RaiseClick();
};

}
