#pragma once

#include <string>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
    class KeyboardEventArgs;
}

namespace NeoWPF::UI {

class TextBlock : public FrameworkElement {
public:
    TextBlock();
    ~TextBlock();

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }
    int GetFontSize() const noexcept { return m_fontSize; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;
    void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) override;

    bool IsInteractable() const noexcept { return m_interactable; }
    void SetInteractable(bool interactable) noexcept { m_interactable = interactable; }

private:
    std::wstring m_text;
    uint32_t m_textColor = 0xFF000000u;
    int m_fontSize = 16;
    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    bool m_textDirty = true;
    bool m_interactable = false;

    void EnsureTextTexture();
};

}
