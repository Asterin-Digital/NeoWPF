#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>

namespace NeoWPF::UI {

Popup::Popup()
{
    SetPresenter(std::make_shared<Content::ContentPresenter>());
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Debug,
        "Constructed popup=%p", this);
}
Popup::~Popup()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Debug,
        "Destroyed popup=%p hostWindow=%p", this, m_hostWindow);
}

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
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Info,
        "SetIsOpen popup=%p requested=%d current=%d applying=%d target=%p",
        this, isOpen ? 1 : 0, GetIsOpen() ? 1 : 0,
        IsApplyingPropertyValue() ? 1 : 0, GetPlacementTarget());
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
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Debug,
        "Resolved host popup=%p target=%p window=%p opening=%d",
        this, target, window, isOpen ? 1 : 0);
    const auto self = weak_from_this().lock();
    if (self && window && isOpen) {
        window->AddOverlay(self);
        m_hostWindow = window;
        NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(window->GetOverlays());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
            NeoWPF::Diagnostics::Level::Info,
            "Opened popup=%p overlays=%zu", this, window->GetOverlays().size());
    } else if (self && m_hostWindow && !isOpen) {
        m_hostWindow->RemoveOverlay(self);
        NeoWPF::Input::InputManager::GetInstance().SetOverlayElements(m_hostWindow->GetOverlays());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
            NeoWPF::Diagnostics::Level::Info,
            "Closed popup=%p overlays=%zu", this, m_hostWindow->GetOverlays().size());
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
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Debug,
        "SetPlacementTarget popup=%p target=%p currentOpen=%d",
        this, target, GetIsOpen() ? 1 : 0);
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
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Trace,
        "Measure popup=%p open=%d available=%.1fx%.1f",
        this, GetIsOpen() ? 1 : 0, availableWidth, availableHeight);
    if (!GetIsOpen()) {
        SetDesiredSize(0, 0);
        return;
    }
    Content::ContentControl::Measure(availableWidth, availableHeight);
}

void Popup::Arrange(double finalWidth, double finalHeight)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Trace,
        "Arrange popup=%p open=%d final=%.1fx%.1f target=%p placement=%d offset=%.1f,%.1f",
        this, GetIsOpen() ? 1 : 0, finalWidth, finalHeight,
        GetPlacementTarget(), static_cast<int>(GetPlacement()),
        GetHorizontalOffset(), GetVerticalOffset());
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
        case PopupPlacement::Center:
            x += targetX + (target->GetFinalWidth() - finalWidth) / 2.0;
            y += targetY + (target->GetFinalHeight() - finalHeight) / 2.0;
            break;
        }
    }
    SetLayoutOffset(x, y);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Debug,
        "Arranged popup=%p position=%.1f,%.1f targetFinal=%.1fx%.1f",
        this, x, y,
        GetPlacementTarget() ? GetPlacementTarget()->GetFinalWidth() : 0.0,
        GetPlacementTarget() ? GetPlacementTarget()->GetFinalHeight() : 0.0);
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
