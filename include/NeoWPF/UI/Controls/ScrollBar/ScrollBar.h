#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

class ScrollBarStyle;
void RegisterScrollBarStyles();

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
    void SetTrackColor(NeoWPF::UI::Color color) noexcept { m_trackColor = color; InvalidateVisual(); }
    void SetTrackColor(std::uint32_t color) noexcept { SetTrackColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the current track color.
    */
    NeoWPF::UI::Color GetTrackColor() const noexcept { return m_trackColor; }

    /*
        @summary
        Sets the thumb color.

        @param color
        ARGB color value.
    */
    void SetThumbColor(NeoWPF::UI::Color color) noexcept { m_thumbColor = color; InvalidateVisual(); }
    void SetThumbColor(std::uint32_t color) noexcept { SetThumbColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the current thumb color.
    */
    NeoWPF::UI::Color GetThumbColor() const noexcept { return m_thumbColor; }

    /*
        @summary
        Sets the thumb size in pixels.

        @param size
        A minimum-sized thumb; values below 1 are clamped to 1.
    */
    void SetThumbSize(int size) noexcept { m_thumbSize = size < 1 ? 1 : size; InvalidateVisual(); }

    /*
        @summary
        Returns the current thumb size.
    */
    int GetThumbSize() const noexcept { return m_thumbSize; }

    /*
        @summary
        Updates the total scrollable content size.

        @param size
        Total size of the content in scroll direction.

        @internal
    */
    void SetContentSize(int size) noexcept;

    /*
        @summary
        Updates the visible viewport size.

        @param size
        Visible size of the viewport in scroll direction.

        @internal
    */
    void SetViewportSize(int size) noexcept;

    /*
        @summary
        Sets the current scroll position.

        @param value
        The new value, clamped to the available scroll range.

        @example
        scrollBar->SetValue(240);
    */
    void SetValue(int value) noexcept;

    /*
        @summary
        Returns the current scroll position.
    */
    int GetValue() const noexcept { return static_cast<int>(RangeBase::GetValue()); }

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
    NeoWPF::Core::Event<int> ValueChanged;

    /*
        @summary
        Measures the scrollbar's desired size.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Arranges the scrollbar into its final bounds.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

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
    int GetScrollRange() const noexcept;

    /*
        @summary
        Computes the visual thumb length.

        @internal
    */
    int GetThumbLength(int trackLength) const noexcept;

    /*
        @summary
        Computes the thumb's current position along the track.

        @internal
    */
    int GetThumbPosition(int trackLength, int thumbLength) const noexcept;

private:
    

    /*
        @summary
        Updates the current value based on a pointer position.

        @internal
    */
    void UpdateValueFromPosition(int x, int y) noexcept;

    void OnValueChanged(double value) override;
    void OnPaddingChanged(int padding) override;
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderThicknessChanged(int thickness) override;

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
    NeoWPF::UI::Color m_trackColor = NeoWPF::UI::Color::FromArgb(0xFF2A2A2Au);
    NeoWPF::UI::Color m_thumbColor = NeoWPF::UI::Color::FromArgb(0xFF5A5A5Au);
    int m_thumbSize = 24;
    int m_contentSize = 0;
    int m_viewportSize = 0;
};

class ScrollBarTemplate : public Templates::ControlTemplate {
public:
    ScrollBarTemplate();
};

}
