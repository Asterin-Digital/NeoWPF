#pragma once

#include <string>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Core/Events/Event.h>

namespace NeoWPF::Input::Core {
    class MouseEventArgs;
}

namespace NeoWPF::UI::Styling {
    class Style;  
}

namespace NeoWPF::UI {

class ButtonStyle;  
void RegisterButtonStyles();

/*
    @summary
    A simple button control

    @details
    This is the basic clickable thing you use for actions. It can show text, change colors,
    and raise `Click` when the user presses it.
*/
class Button : public FrameworkElement {
public:
    Button();

    /*
        @summary
        Creates a new button with some starting text.

        @param text
        The text to show inside the button.
    */
    Button(const std::wstring& text);

    ~Button();

    /*
        @summary
        Replaces the button text.

        @param text
        The new text to display.
    */
    void SetText(const std::wstring& text);

    /*
        @summary
        Returns the current button text.
    */
    const std::wstring& GetText() const noexcept { return m_text; }

    /*
        @summary
        Sets the text color.

        @param color
        ARGB color value.
    */
    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }

    /*
        @summary
        Returns the current text color.
    */
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    /*
        @summary
        Sets the font size used for the button text.

        @param fontSize
        Size in pixels.
    */
    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }

    /*
        @summary
        Returns the current font size.
    */
    int GetFontSize() const noexcept { return m_fontSize; }

    /*
        @summary
        Sets the padding around the text inside the button.

        @param padding
        Padding in pixels.
    */
    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }

    /*
        @summary
        Returns the current padding.
    */
    int GetPadding() const noexcept { return m_padding; }

    /*
        @summary
        Sets the background color.

        @param color
        ARGB color value.
    */
    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }

    /*
        @summary
        Returns the current background color.
    */
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

    /*
        @summary
        Measures the button and decides how much space it needs.

        @param availableWidth
        Width available for layout.

        @param availableHeight
        Height available for layout.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Positions the button inside its box

        @param finalWidth
        The width for the button.

        @param finalHeight
        The height for the button.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Builds the render commands for the button.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Controls whether the button can be hit by mouse input.
    */
    bool IsHitTestVisible() const noexcept override { return true; }

    /*
        @summary
        Returns whether the button is currently interactable.
    */
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    /*
        @summary
        Handles mouse-up input.

        @param x
        The x position of the mouse.

        @param y
        The y position of the mouse.

        @param buttonState
        The mouse button state.

        @internal
    */
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    /*
        @summary
        Raised when the button is clicked.
    */
    NeoWPF::Core::Event<> Click;

private:
    std::wstring m_text;
    uint32_t m_textColor = 0xFF000000u;
    uint32_t m_backgroundColor = 0xFF2F2F2Fu;
    int m_fontSize = 16;

private:
    int m_padding = 8;
    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    bool m_textDirty = true;

    /*
        @summary
        Makes sure the text texture exists before drawing.

        @internal
    */
    void EnsureTextTexture();

    /*
        @summary
        Raises the Click event.

        @internal
    */
    void RaiseClick();
};

}
