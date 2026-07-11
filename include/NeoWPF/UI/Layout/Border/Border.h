#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <memory>

namespace NeoWPF::UI {

/*
    @summary
    Hosts a single child with optional padding, border, and background.

    @details
    The border reserves interior space for its content so the child stays inside the visible border area.
*/
void RegisterBorderStyles();

class Border : public NeoWPF::UI::Content::ContentControl {
public:
    /*
        @summary
        Creates a border with default empty content and styling.
    */
    Border();

    using ContentControl::SetContent;
    using ContentControl::GetContent;
    using ContentControl::ClearContent;

    /*
        @summary
        Sets the single child hosted by the border.

        @param child
        The content to place inside the border.
    */
    void SetChild(const std::shared_ptr<NeoWPF::UI::UIElement>& child) { SetContent(child); }

    /*
        @summary
        Gets the child currently hosted by the border.
    */
    const std::shared_ptr<NeoWPF::UI::UIElement>& GetChild() const noexcept { return GetContent(); }

    /*
        @summary
        Removes the child currently hosted by the border.
    */
    void ClearChild() { ClearContent(); }

    /*
        @summary
        Sets the space between the border and its content.

        @param padding
        The inset size in pixels.
    */
    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }

    /*
        @summary
        Gets the current padding.
    */
    int GetPadding() const noexcept { return m_padding; }

    /*
        @summary
        Sets the border thickness.

        @param thickness
        The border width in pixels.
    */
    void SetBorderThickness(int thickness) noexcept { m_borderThickness = thickness; InvalidateVisual(); }

    /*
        @summary
        Gets the current border thickness.
    */
    int GetBorderThickness() const noexcept { return m_borderThickness; }

    /*
        @summary
        Sets the color used to draw the border.

        @param color
        The border color in ARGB format.
    */
    void SetBorderColor(NeoWPF::UI::Color color) noexcept { m_borderColor = color; InvalidateVisual(); }
    void SetBorderColor(std::uint32_t color) noexcept { SetBorderColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Gets the border color.
    */
    NeoWPF::UI::Color GetBorderColor() const noexcept { return m_borderColor; }

    /*
        @summary
        Sets the background color drawn inside the border.

        @param color
        The background color in ARGB format.
    */
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Gets the background color.
    */
    NeoWPF::UI::Color GetBackgroundColor() const noexcept { return m_backgroundColor; }

    /*
        @summary
        Measures the border and its content using the available space.

        @param availableWidth
        The width available to the border.

        @param availableHeight
        The height available to the border.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Positions the border and its content inside the assigned bounds.

        @param finalWidth
        The width assigned to the border.

        @param finalHeight
        The height assigned to the border.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    int m_padding = 0;
    int m_borderThickness = 0;
    NeoWPF::UI::Color m_borderColor = NeoWPF::UI::Color::FromArgb(0xFF000000u);
    NeoWPF::UI::Color m_backgroundColor = NeoWPF::UI::Color::FromArgb(0x00000000u);
};

}
