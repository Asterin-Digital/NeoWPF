#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/Input/InputManager.h>

namespace NeoWPF::UI {

Popup::Popup()
{
    SetPresenter(std::make_shared<Content::ContentPresenter>());
}
Popup::~Popup() = default;

bool Popup::GetIsOpen() const noexcept
{
    try {
        return GetValue<bool>(IsOpenProperty());
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

void Popup::SetIsOpen(bool isOpen)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(IsOpenProperty(), isOpen);
        return;
    }
    auto* target = GetPlacementTarget();
    auto* window = dynamic_cast<Window*>(target);
    while (!window && target) {
        target = dynamic_cast<FrameworkElement*>(target->GetParent());
        window = dynamic_cast<Window*>(target);
    }
    const auto self = weak_from_this().lock();
    if (self && window && isOpen) {
        window->AddOverlay(self);
        m_hostWindow = window;
        NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(window->GetOverlays());
    } else if (self && m_hostWindow && !isOpen) {
        m_hostWindow->RemoveOverlay(self);
        NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(m_hostWindow->GetOverlays());
    }
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

FrameworkElement* Popup::GetPlacementTarget() const noexcept
{
    try {
        return GetValue<FrameworkElement*>(PlacementTargetProperty());
    } catch (const std::bad_any_cast&) {
        return nullptr;
    }
}

void Popup::SetPlacementTarget(FrameworkElement* target)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(PlacementTargetProperty(), target);
        return;
    }
    if (GetIsOpen()) {
        InvalidateArrange();
        InvalidateVisual();
    }
}

bool Popup::GetStaysOpen() const noexcept
{
    try {
        return GetValue<bool>(StaysOpenProperty());
    } catch (const std::bad_any_cast&) {
        return true;
    }
}

void Popup::SetStaysOpen(bool staysOpen)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(StaysOpenProperty(), staysOpen);
    }
}

PopupPlacement Popup::GetPlacement() const noexcept
{
    try {
        return GetValue<PopupPlacement>(PlacementProperty());
    } catch (const std::bad_any_cast&) {
        return PopupPlacement::Bottom;
    }
}

void Popup::SetPlacement(PopupPlacement placement)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(PlacementProperty(), placement);
    }
}

double Popup::GetHorizontalOffset() const noexcept
{
    try {
        return GetValue<double>(HorizontalOffsetProperty());
    } catch (const std::bad_any_cast&) {
        return 0;
    }
}

void Popup::SetHorizontalOffset(double offset)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(HorizontalOffsetProperty(), offset);
    }
}

double Popup::GetVerticalOffset() const noexcept
{
    try {
        return GetValue<double>(VerticalOffsetProperty());
    } catch (const std::bad_any_cast&) {
        return 0;
    }
}

void Popup::SetVerticalOffset(double offset)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(VerticalOffsetProperty(), offset);
    }
}

bool Popup::IsHitTestVisible() const noexcept
{
    return GetIsOpen() && Content::ContentControl::IsHitTestVisible();
}

void Popup::Measure(double availableWidth, double availableHeight)
{
    if (!GetIsOpen()) {
        SetDesiredSize(0, 0);
        return;
    }
    Content::ContentControl::Measure(availableWidth, availableHeight);
}

void Popup::Arrange(double finalWidth, double finalHeight)
{
    if (!GetIsOpen()) {
        return;
    }
    double x = GetHorizontalOffset();
    double y = GetVerticalOffset();
    if (auto* target = GetPlacementTarget()) {
        double targetX = 0.0;
        double targetY = 0.0;
        target->GetAbsolutePosition(targetX, targetY);
        switch (GetPlacement()) {
        case PopupPlacement::Bottom:
            x += targetX;
            y += targetY + target->GetFinalHeight();
            break;
        case PopupPlacement::Top:
            x += targetX;
            y += targetY - finalHeight;
            break;
        case PopupPlacement::Left:
            x += targetX - finalWidth;
            y += targetY;
            break;
        case PopupPlacement::Right:
            x += targetX + target->GetFinalWidth();
            y += targetY;
            break;
        }
    }
    SetLayoutOffset(x, y);
    Content::ContentControl::Arrange(finalWidth, finalHeight);
}

void Popup::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!GetIsOpen()) {
        return;
    }
    Content::ContentControl::BuildRenderCommands(commands);
}

}
