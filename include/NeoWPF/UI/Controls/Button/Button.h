#pragma once

#include <memory>
#include <string>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Color.h>

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
class Button : public NeoWPF::UI::ButtonBase {
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
    void SetTextColor(NeoWPF::UI::Color color) noexcept { m_textColor = color; if (m_textContent) { m_textContent->SetTextColor(color.ToArgb()); } InvalidateVisual(); }

    /*
        @summary
        Returns the current text color.
    */
    NeoWPF::UI::Color GetTextColor() const noexcept { return m_textColor; }

    /*
        @summary
        Sets the font size used for the button text.

        @param fontSize
        Size in pixels.
    */
    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; if (m_textContent) { m_textContent->SetFontSize(fontSize); } InvalidateVisual(); }

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
    void SetPadding(int padding) noexcept { m_padding = padding; if (auto presenter = GetPresenter()) { presenter->SetPadding(padding); } InvalidateVisual(); }

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
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the current background color.
    */
    NeoWPF::UI::Color GetBackgroundColor() const noexcept { return m_backgroundColor; }

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

    void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;
    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;

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

protected:
    using ButtonBase::InitializeVisualStates;
    using ButtonBase::ApplyVisualState;
    using ButtonBase::RaiseClick;

private:
    std::wstring m_text;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFF000000u);
    NeoWPF::UI::Color m_backgroundColor = NeoWPF::UI::Color::FromArgb(0xFF2F2F2Fu);
    NeoWPF::UI::Color m_baseBackgroundColor = NeoWPF::UI::Color::FromArgb(0xFF2F2F2Fu);
    int m_fontSize = 16;

private:
    int m_padding = 8;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;

    void OnVisualStateApplied(const std::string& stateName) override;
};

class ButtonTemplate : public Templates::ControlTemplate {
public:
    ButtonTemplate();
};

}
