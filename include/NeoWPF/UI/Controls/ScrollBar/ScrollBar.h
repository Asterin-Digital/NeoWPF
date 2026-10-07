/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

/*
    @summary
    A scrollbar for vertical or horizontal scrolling.

    @details
    Drop this on a panel or content surface when you want scrolling.
    @example
    auto scrollBar = std::make_shared<NeoWPF::UI::ScrollBar>();
    scrollBar->SetOrientation(Orientation::Vertical);
    scrollBar->SetScrollTarget(contentHost);
    scrollBar->SetViewportElement(viewport);
*/
class ScrollBar : public NeoWPF::UI::RangeBase {
public:
    static const NeoWPF::UI::Properties::DependencyProperty& TrackColorProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ThumbColorProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& ThumbSizeProperty();

    ScrollBar();

    ~ScrollBar();

    /*
        @summary
        Chooses whether the scrollbar runs vertically or horizontally.

        @param orientation
        The direction the thumb should travel.

        @example
        scrollBar->SetOrientation(Orientation::Horizontal);
    */
    void SetOrientation(Orientation orientation) noexcept;

    /*
        @summary
        Returns the current scrollbar direction.
    */
    Orientation GetOrientation() const noexcept { return m_orientation; }

    /*
        @summary
        Sets the track color behind the thumb.

        @param color
        ARGB color value.
    */
    void SetTrackColor(NeoWPF::UI::Color color) noexcept;
    void SetTrackColor(std::uint32_t color) noexcept { SetTrackColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the current track color.
    */
    NeoWPF::UI::Color GetTrackColor() const noexcept;

    /*
        @summary
        Sets the thumb color.

        @param color
        ARGB color value.
    */
    void SetThumbColor(NeoWPF::UI::Color color) noexcept;
    void SetThumbColor(std::uint32_t color) noexcept { SetThumbColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the current thumb color.
    */
    NeoWPF::UI::Color GetThumbColor() const noexcept;

    /*
        @summary
        Sets the thumb size in pixels.

        @param size
        A minimum-sized thumb; values below 1 are clamped to 1.
    */
    void SetThumbSize(double size) noexcept;

    /*
        @summary
        Returns the current thumb size.
    */
    double GetThumbSize() const noexcept;

    /*
        @summary
        Updates the total scrollable content size.

        @param size
        Total size of the content in scroll direction.

        @internal
    */
    void SetContentSize(double size) noexcept;

    /*
        @summary
        Updates the visible viewport size.

        @param size
        Visible size of the viewport in scroll direction.

        @internal
    */
    void SetViewportSize(double size) noexcept;

    /*
        @summary
        Sets the current scroll position.

        @param value
        The new value, clamped to the available scroll range.

        @example
        scrollBar->SetValue(240);
    */
    void SetValue(double value) noexcept;

    /*
        @summary
        Returns the current scroll position.
    */
    double GetValue() const noexcept { return RangeBase::GetValue(); }

    void LineUp() noexcept;
    void LineDown() noexcept;
    void PageUp() noexcept;
    void PageDown() noexcept;

    /*
        @summary
        Raised whenever the scroll position changes.

        @details
        Hook this up if you want to react to the thumb moving, such as updating other UI.
    */
    NeoWPF::Core::Event<double> ValueChanged;

    /*
        @summary
        Measures the scrollbar's desired size.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the scrollbar into its final bounds.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Builds the render commands for the track and thumb.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    
    void OnInitialize() override;

    /*
        @summary
        Computes the available scroll range.

        @internal
    */
    double GetScrollRange() const noexcept;

    /*
        @summary
        Computes the visual thumb length.

        @internal
    */
    double GetThumbLength(double trackLength) const noexcept;

    /*
        @summary
        Computes the thumb's current position along the track.

        @internal
    */
    double GetThumbPosition(double trackLength, double thumbLength) const noexcept;

private:
    

    /*
        @summary
        Updates the current value based on a pointer position.

        @internal
    */
    void UpdateValueFromPosition(double x, double y) noexcept;

    void OnValueChanged(double value) override;
    void OnPaddingChanged(Thickness padding) override;
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderThicknessChanged(double thickness) override;

    /*
        @summary
        Recomputes content and viewport size, then updates the target.

        @internal
    */
    void UpdateScrollRange() noexcept;
    void UpdateScrollState() noexcept;
    void UpdateThumbVisualState() noexcept;

public:
    int GetOrientationValue() const noexcept;
    void SetOrientationValue(int orientation) noexcept;
    static const NeoWPF::UI::Properties::DependencyProperty& GetOrientationProperty();

private:

    Orientation m_orientation = Orientation::Vertical;
    double m_contentSize = 0.0;
    double m_viewportSize = 0.0;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

class ScrollBarTemplate : public Templates::ControlTemplate {
public:
    ScrollBarTemplate();
};

}
