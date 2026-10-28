#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Keyboard.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <limits>

namespace NeoWPF::UI {

namespace {

void ApplySliderTheme(Slider& slider)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeBorderThickness;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;
    using NeoWPF::Core::ApplicationThemes::ThemeMetric;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;

    auto& themeManager = ThemeManager::Instance();
    slider.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Track));
    slider.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Border));

    double borderThickness = 0.0;
    if (themeManager.TryGetMetric(ThemeMetric::SliderBorderThickness, borderThickness)) {
        slider.SetThemePropertyValue("BorderThickness", borderThickness);
    }

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::Slider, padding)) {
        slider.SetThemePropertyValue("Padding", padding);
    }
}

}

Slider::Slider()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<SliderTemplate>());
    ApplySliderTheme(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplySliderTheme(*this);
    });
}

Slider::~Slider()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
}

void Slider::SetOrientation(Orientation orientation) noexcept
{
    static_cast<Properties::DependencyObject&>(*this).SetValue(
        GetOrientationProperty(), static_cast<int>(orientation));
}

void Slider::SetIsDirectionReversed(bool value) noexcept
{
    m_isDirectionReversed = value;
    InvalidateVisual();
    UpdateThumbVisualState();
}

void Slider::SetIsSnapToTickEnabled(bool value) noexcept
{
    m_isSnapToTickEnabled = value;
    SetValue(GetValue());
}

void Slider::SetTickFrequency(double value) noexcept
{
    m_tickFrequency = std::max(0.0, value);
    SetValue(GetValue());
}

void Slider::SetTickPlacement(TickPlacement placement) noexcept
{
    m_tickPlacement = placement;
    InvalidateVisual();
}

void Slider::SetTicks(const Core::DoubleCollection& ticks)
{
    m_ticks = ticks;
    SetValue(GetValue());
    InvalidateVisual();
}

void Slider::SetIsSelectionRangeEnabled(bool value) noexcept
{
    m_isSelectionRangeEnabled = value;
    InvalidateVisual();
}

void Slider::SetSelectionStart(double value) noexcept
{
    m_selectionStart = std::clamp(value, GetMinimum(), GetMaximum());
    InvalidateVisual();
}

void Slider::SetSelectionEnd(double value) noexcept
{
    m_selectionEnd = std::clamp(value, GetMinimum(), GetMaximum());
    InvalidateVisual();
}

void Slider::Measure(double availableWidth, double availableHeight)
{
    constexpr double thickness = 16.0;
    constexpr double minimumLength = 24.0;
    const bool isVertical = m_orientation == Orientation::Vertical;
    const double primaryAvailable = std::max(0.0, isVertical ? availableHeight : availableWidth);
    const double length = std::max(minimumLength, primaryAvailable);

    if (isVertical) {
        SetDesiredSize(thickness, length);
    } else {
        SetDesiredSize(length, thickness);
    }
}

void Slider::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateThumbVisualState();
}

void Slider::OnInitialize()
{
    UIElement::OnInitialize();

    if (auto thumb = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumb->DragDelta += [this](int delta) {
            const double trackLength = GetTrackLength();
            const double thumbLength = GetThumbLength();
            const double availableLength = std::max(0.0, trackLength - thumbLength);
            const double range = GetMaximum() - GetMinimum();
            if (availableLength <= 0.0 || range <= 0.0) {
                return;
            }

            const double direction = m_isDirectionReversed ? -1.0 : 1.0;
            SetValue(GetValue() + direction * delta * range / availableLength);
        };
    }
}

void Slider::OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args) {
        return;
    }

    auto* keyboardArgs = dynamic_cast<NeoWPF::Input::Core::RoutedKeyboardEventArgs*>(args);
    auto* pointerArgs = dynamic_cast<NeoWPF::Input::Core::RoutedPointerEventArgs*>(args);
    if (pointerArgs && args->GetRoutedEvent() == &NeoWPF::Input::Core::PointerPressedEvent()) {
        if (pointerArgs->GetChangedButton() != 0x01u || !IsInteractable()) {
            return;
        }

        Focus();
        const auto thumb = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"));
        if (thumb && args->GetOriginalSource() == thumb.get()) {
            return;
        }

        double absoluteX = 0.0;
        double absoluteY = 0.0;
        GetAbsolutePosition(absoluteX, absoluteY);
        SetValue(ValueFromPosition(pointerArgs->GetX() - absoluteX,
                                   pointerArgs->GetY() - absoluteY));
        return;
    }

    if (!keyboardArgs || args->GetRoutedEvent() != &NeoWPF::Input::Core::KeyDownEvent() ||
        !keyboardArgs->IsKeyDown() || !IsInteractable()) {
        return;
    }

    const auto key = static_cast<NeoWPF::Input::Key>(keyboardArgs->GetKeyCode());
    const bool isVertical = m_orientation == Orientation::Vertical;
    const bool increaseKey = isVertical ? key == NeoWPF::Input::Key::Up : key == NeoWPF::Input::Key::Right;
    const bool decreaseKey = isVertical ? key == NeoWPF::Input::Key::Down : key == NeoWPF::Input::Key::Left;

    if (increaseKey) {
        SetValue(GetValue() + (m_isDirectionReversed ? -1.0 : 1.0) * GetSmallChange());
    } else if (decreaseKey) {
        SetValue(GetValue() - (m_isDirectionReversed ? -1.0 : 1.0) * GetSmallChange());
    } else if (key == NeoWPF::Input::Key::PageUp) {
        SetValue(GetValue() + GetLargeChange());
    } else if (key == NeoWPF::Input::Key::PageDown) {
        SetValue(GetValue() - GetLargeChange());
    } else if (key == NeoWPF::Input::Key::Home) {
        SetValue(GetMinimum());
    } else if (key == NeoWPF::Input::Key::End) {
        SetValue(GetMaximum());
    }
}

void Slider::OnValueChanged(double value)
{
    const double snapped = CoerceValueToTicks(value);
    if (snapped != value) {
        RangeBase::SetValue(snapped);
        return;
    }

    RangeBase::OnValueChanged(value);
    UpdateThumbVisualState();
}

void Slider::OnPaddingChanged(Thickness)
{
    InvalidateVisual();
}

void Slider::OnBorderThicknessChanged(double)
{
    InvalidateVisual();
}

double Slider::CoerceValueToTicks(double value) const noexcept
{
    if (!m_isSnapToTickEnabled) {
        return value;
    }

    double nearest = value;
    double distance = std::numeric_limits<double>::max();
    const auto consider = [&](double tick) {
        const double clamped = std::clamp(tick, GetMinimum(), GetMaximum());
        const double candidateDistance = std::abs(clamped - value);
        if (candidateDistance < distance) {
            distance = candidateDistance;
            nearest = clamped;
        }
    };

    if (m_tickFrequency > 0.0) {
        const double first = GetMinimum();
        const double index = std::round((value - first) / m_tickFrequency);
        consider(first + index * m_tickFrequency);
    }

    for (const double tick : m_ticks) {
        consider(tick);
    }

    return nearest;
}

double Slider::GetTrackLength() const noexcept
{
    return m_orientation == Orientation::Vertical ? GetFinalHeight() : GetFinalWidth();
}

double Slider::GetThumbLength() const noexcept
{
    double thumbLength = 16.0;
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().TryGetMetric(
        NeoWPF::Core::ApplicationThemes::ThemeMetric::SliderThumbLength,
        thumbLength);
    return thumbLength;
}

double Slider::GetThumbPosition() const noexcept
{
    const double availableLength = std::max(0.0, GetTrackLength() - GetThumbLength());
    const double range = GetMaximum() - GetMinimum();
    if (availableLength <= 0.0 || range <= 0.0) {
        return 0.0;
    }

    const double ratio = (GetValue() - GetMinimum()) / range;
    const double effectiveRatio = m_isDirectionReversed ? 1.0 - ratio : ratio;
    return std::clamp(effectiveRatio, 0.0, 1.0) * availableLength;
}

double Slider::ValueFromPosition(double x, double y) const noexcept
{
    const double availableLength = std::max(0.0, GetTrackLength() - GetThumbLength());
    const double range = GetMaximum() - GetMinimum();
    if (availableLength <= 0.0 || range <= 0.0) {
        return GetMinimum();
    }

    const double pointer = m_orientation == Orientation::Vertical ? y : x;
    const double ratio = std::clamp((pointer - GetThumbLength() / 2.0) / availableLength, 0.0, 1.0);
    const double effectiveRatio = m_isDirectionReversed ? 1.0 - ratio : ratio;
    return GetMinimum() + effectiveRatio * range;
}

void Slider::UpdateThumbVisualState() noexcept
{
    if (auto thumb = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        const bool isVertical = m_orientation == Orientation::Vertical;
        const double length = GetThumbLength();
        const double position = GetThumbPosition();
        const double thickness = isVertical ? GetFinalWidth() : GetFinalHeight();
        thumb->SetOrientation(m_orientation);
        thumb->SetBounds(
            static_cast<int>(std::lround(isVertical ? 0.0 : position)),
            static_cast<int>(std::lround(isVertical ? position : 0.0)),
            static_cast<int>(std::lround(isVertical ? thickness : length)),
            static_cast<int>(std::lround(isVertical ? length : thickness)));
    }
}

void Slider::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto transform = GetWorldRenderTransform();
    double absX = 0.0;
    double absY = 0.0;
    transform.positionMatrix.TransformPoint(0.0f, 0.0f, absX, absY);
    const auto clip = GetClipRect();
    const bool isVertical = m_orientation == Orientation::Vertical;
    const double trackLength = GetTrackLength();
    const double trackThickness = isVertical ? GetFinalWidth() : GetFinalHeight();
    const double thumbLength = GetThumbLength();
    const double thumbPosition = GetThumbPosition();

    auto track = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Track"));
    auto thumb = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Thumb"));

    UpdateThumbVisualState();

    if (track) {
        track->SetRect(absX, absY, isVertical ? trackThickness : trackLength, isVertical ? trackLength : trackThickness);
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);
    }

    if (auto thumbControl = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumbControl->InvalidateVisual();
        thumbControl->BuildRenderCommands(commands);
    } else if (thumb) {
        thumb->SetRect(absX + (isVertical ? 0.0 : thumbPosition), absY + (isVertical ? thumbPosition : 0.0),
                       isVertical ? trackThickness : thumbLength, isVertical ? thumbLength : trackThickness);
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    }
}

int Slider::GetOrientationValue() const noexcept
{
    return static_cast<int>(m_orientation);
}

void Slider::SetOrientationValue(int orientation) noexcept
{
    m_orientation = static_cast<Orientation>(orientation);
    if (auto thumb = std::dynamic_pointer_cast<Thumb>(FindTemplateChild("PART_Thumb"))) {
        thumb->SetOrientation(m_orientation);
    }
    InvalidateMeasure();
    InvalidateVisual();
}

}