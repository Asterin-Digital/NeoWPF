/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Slider/TickPlacement.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Collections/DoubleCollection.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

class SliderTemplate;

/*
    @summary
    Provides a control for selecting a numeric value from a continuous range.

    @details
    The slider supports horizontal and vertical layouts, tick snapping, and an
    optional selection range.

    @example
    auto slider = std::make_shared<NeoWPF::UI::Slider>();
    slider->SetMinimum(0.0);
    slider->SetMaximum(100.0);
    slider->SetValue(50.0);
    slider->ValueChanged += [](double value) {
        // Respond to the selected value.
    };
*/
class Slider : public RangeBase {
public:

    Slider();

    ~Slider() override;

    /*
        @summary
        Sets the direction in which the slider track is laid out.

        @param orientation
        The desired slider orientation.
    */
    void SetOrientation(Orientation orientation) noexcept;

    /*
        @summary
        Returns the slider orientation.

        @returns
        The current slider orientation.
    */
    Orientation GetOrientation() const noexcept { return m_orientation; }

    /*
        @summary
        Enables or disables reversed value direction along the track.

        @param value
        True to reverse the value direction.
    */
    void SetIsDirectionReversed(bool value) noexcept;

    /*
        @summary
        Returns whether the slider value direction is reversed.

        @returns
        True when the value direction is reversed.
    */
    bool GetIsDirectionReversed() const noexcept { return m_isDirectionReversed; }

    /*
        @summary
        Enables or disables snapping values to ticks.

        @param value
        True to enable tick snapping.
    */
    void SetIsSnapToTickEnabled(bool value) noexcept;

    /*
        @summary
        Returns whether tick snapping is enabled.

        @returns
        True when values snap to ticks.
    */
    bool GetIsSnapToTickEnabled() const noexcept { return m_isSnapToTickEnabled; }

    /*
        @summary
        Sets the distance between generated ticks.

        @param value
        The distance between generated ticks.
    */
    void SetTickFrequency(double value) noexcept;

    /*
        @summary
        Returns the distance between generated ticks.

        @returns
        The configured tick frequency.
    */
    double GetTickFrequency() const noexcept { return m_tickFrequency; }

    /*
        @summary
        Sets the placement of tick marks relative to the track.

        @param placement
        The desired tick placement.
    */
    void SetTickPlacement(TickPlacement placement) noexcept;

    /*
        @summary
        Returns the placement of tick marks relative to the track.

        @returns
        The configured tick placement.
    */
    TickPlacement GetTickPlacement() const noexcept { return m_tickPlacement; }

    /*
        @summary
        Replaces the explicit tick values used for snapping.

        @param ticks
        The explicit tick values.
    */
    void SetTicks(const Core::DoubleCollection& ticks);

    /*
        @summary
        Returns the explicit tick values used for snapping.

        @returns
        The configured explicit tick values.
    */
    const Core::DoubleCollection& GetTicks() const noexcept { return m_ticks; }

    /*
        @summary
        Enables or disables the selection range rendered on the track.

        @param value
        True to enable the selection range.
    */
    void SetIsSelectionRangeEnabled(bool value) noexcept;

    /*
        @summary
        Returns whether the selection range is enabled.

        @returns
        True when the selection range is enabled.
    */
    bool GetIsSelectionRangeEnabled() const noexcept { return m_isSelectionRangeEnabled; }

    /*
        @summary
        Sets the beginning of the selection range.

        @param value
        The desired selection start value.
    */
    void SetSelectionStart(double value) noexcept;

    /*
        @summary
        Returns the beginning of the selection range.

        @returns
        The selection start value.
    */
    double GetSelectionStart() const noexcept { return m_selectionStart; }

    /*
        @summary
        Sets the end of the selection range.

        @param value
        The desired selection end value.
    */
    void SetSelectionEnd(double value) noexcept;

    /*
        @summary
        Returns the end of the selection range.

        @returns
        The selection end value.
    */
    double GetSelectionEnd() const noexcept { return m_selectionEnd; }

    /*
        @summary
        Measures the slider using the available layout space.

        @param availableWidth
        Maximum width available.

        @param availableHeight
        Maximum height available.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the slider within its assigned layout space.

        @param finalWidth
        Width assigned to the slider.

        @param finalHeight
        Height assigned to the slider.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Appends the slider track and thumb rendering commands.

        @param commands
        Command list receiving the slider rendering commands.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Initializes the slider's template interaction behavior.

        @internal
    */
    void OnInitialize() override;

    /*
        @summary
        Handles a mouse press on the slider.

        @param x
        Mouse position on the window's horizontal axis.

        @param y
        Mouse position on the window's vertical axis.

        @param buttonState
        Current mouse button state flags.

        @internal
    */
    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;

    void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) override;

    int GetOrientationValue() const noexcept;
    void SetOrientationValue(int orientation) noexcept;
    static const NeoWPF::UI::Properties::DependencyProperty& GetOrientationProperty();

    int GetTickPlacementValue() const noexcept { return static_cast<int>(m_tickPlacement); }
    void SetTickPlacementValue(int placement) noexcept { SetTickPlacement(static_cast<TickPlacement>(placement)); }

    static const NeoWPF::UI::Properties::DependencyProperty& IsDirectionReversedProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& IsSnapToTickEnabledProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& TickFrequencyProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& TickPlacementProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& TicksProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& IsSelectionRangeEnabledProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& SelectionStartProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& SelectionEndProperty();

private:
    void OnValueChanged(double value) override;
    void OnPaddingChanged(Thickness padding) override;
    void OnBorderThicknessChanged(double thickness) override;

    double CoerceValueToTicks(double value) const noexcept;
    double ValueFromPosition(double x, double y) const noexcept;
    double GetTrackLength() const noexcept;
    double GetThumbLength() const noexcept;
    double GetThumbPosition() const noexcept;
    void UpdateThumbVisualState() noexcept;

    Orientation m_orientation = Orientation::Horizontal;
    bool m_isDirectionReversed = false;
    bool m_isSnapToTickEnabled = false;
    double m_tickFrequency = 1.0;
    TickPlacement m_tickPlacement = TickPlacement::None;
    Core::DoubleCollection m_ticks;
    bool m_isSelectionRangeEnabled = false;
    double m_selectionStart = 0.0;
    double m_selectionEnd = 0.0;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

class SliderTemplate : public Templates::ControlTemplate {
public:
    SliderTemplate();
};

}