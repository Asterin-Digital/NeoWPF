/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Controls/Slider/TickPlacement.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Collections/DoubleCollection.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

class SliderTemplate;

class Slider : public RangeBase {
public:
    Slider();
    ~Slider() override;

    void SetOrientation(Orientation orientation) noexcept;
    Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetIsDirectionReversed(bool value) noexcept;
    bool GetIsDirectionReversed() const noexcept { return m_isDirectionReversed; }

    void SetIsSnapToTickEnabled(bool value) noexcept;
    bool GetIsSnapToTickEnabled() const noexcept { return m_isSnapToTickEnabled; }

    void SetTickFrequency(double value) noexcept;
    double GetTickFrequency() const noexcept { return m_tickFrequency; }

    void SetTickPlacement(TickPlacement placement) noexcept;
    TickPlacement GetTickPlacement() const noexcept { return m_tickPlacement; }

    void SetTicks(const Core::DoubleCollection& ticks);
    const Core::DoubleCollection& GetTicks() const noexcept { return m_ticks; }

    void SetIsSelectionRangeEnabled(bool value) noexcept;
    bool GetIsSelectionRangeEnabled() const noexcept { return m_isSelectionRangeEnabled; }

    void SetSelectionStart(double value) noexcept;
    double GetSelectionStart() const noexcept { return m_selectionStart; }

    void SetSelectionEnd(double value) noexcept;
    double GetSelectionEnd() const noexcept { return m_selectionEnd; }

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;
    void OnInitialize() override;

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
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
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
};

class SliderTemplate : public Templates::ControlTemplate {
public:
    SliderTemplate();
};

}