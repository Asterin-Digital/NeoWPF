#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <memory>

namespace NeoWPF::UI {

class UIElement;

/*
    @summary
    Hosts a single child with optional padding, border, and background.

    @details
    The border reserves interior space for its content so the child stays inside the visible border area.
*/
void RegisterBorderStyles();

class Border : public NeoWPF::UI::Content::ContentControl {
public:
    static const NeoWPF::UI::Properties::DependencyProperty& PaddingProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& BorderThicknessProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& BorderColorProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& BackgroundColorProperty();

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

    void SetPadding(Thickness padding) noexcept;
    Thickness GetPadding() const noexcept;

    void SetBorderThickness(int thickness) noexcept;
    int GetBorderThickness() const noexcept;
    void SetBorderThickness(Thickness thickness) noexcept;
    Thickness GetBorderThicknessValue() const noexcept;

    void SetBorderColor(NeoWPF::UI::Color color) noexcept;
    void SetBorderColor(std::uint32_t color) noexcept { SetBorderColor(NeoWPF::UI::Color::FromArgb(color)); }

    NeoWPF::UI::Color GetBorderColor() const noexcept;

    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }

    NeoWPF::UI::Color GetBackgroundColor() const noexcept;

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

public:

protected:
    void OnPaddingChanged(Thickness padding) override;
    void OnBorderThicknessChanged(int thickness) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;

private:
    Thickness m_borderThicknesses{};
};

}
