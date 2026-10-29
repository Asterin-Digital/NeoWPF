/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <cstddef>
#include <memory>

namespace NeoWPF::UI {

class UIElement;

/*
    @summary
    Hosts a single child with optional padding, border, and background.

    @details
    The border reserves interior space for its content so the child stays inside the visible border area.
*/
class Border : public NeoWPF::UI::Content::ContentControl {
public:
    static const NeoWPF::UI::Properties::DependencyProperty& PaddingProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& BorderThicknessProperty();

    /*
        @summary
        Creates a border with default empty content and styling.
    */
    Border();
    ~Border() override;

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
    std::shared_ptr<NeoWPF::UI::UIElement> GetChild() const noexcept { return GetContentElement(); }

    /*
        @summary
        Removes the child currently hosted by the border.
    */
    void ClearChild() { ClearContent(); }

    /*
        @summary
        Sets the space between the border and its child.
    */
    void SetPadding(Thickness padding) noexcept;
    /*
        @summary
        Returns the space between the border and its child.
    */
    Thickness GetPadding() const noexcept;

    /*
        @summary
        Sets a uniform border thickness.
    */
    void SetBorderThickness(double thickness) noexcept;
    /*
        @summary
        Returns the uniform border thickness.
    */
    double GetBorderThickness() const noexcept;
    /*
        @summary
        Sets the thickness of each border edge.
    */
    void SetBorderThickness(Thickness thickness) noexcept;
    /*
        @summary
        Returns the thickness of each border edge.
    */
    Thickness GetBorderThicknessValue() const noexcept;

    /*
        @summary
        Measures the border and its content using the available space.

        @param availableWidth
        The width available to the border.

        @param availableHeight
        The height available to the border.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Positions the border and its content inside the assigned bounds.

        @param finalWidth
        The width assigned to the border.

        @param finalHeight
        The height assigned to the border.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Adds border and child drawing operations to a command list.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

public:

protected:
    void OnPaddingChanged(Thickness padding) override;
    void OnBorderThicknessChanged(double thickness) override;
private:
    Thickness m_borderThicknesses{};
    std::size_t m_themeAppliedSubscriptionId = 0;
};

}
