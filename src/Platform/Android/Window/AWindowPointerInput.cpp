#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>

#include <android/input.h>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <typeinfo>

namespace NeoWPF::Platform::Android {
namespace {

using NeoWPF::Input::Core::PointerDeviceType;
using NeoWPF::Input::Core::PointerEventArgs;
using NeoWPF::Input::Core::PointerPhase;

constexpr std::uint32_t PrimaryButton = 0x01u;

PointerDeviceType GetPointerDeviceType(const AInputEvent* event,
                                       std::size_t pointerIndex) noexcept
{
    switch (AMotionEvent_getToolType(event, pointerIndex)) {
    case AMOTION_EVENT_TOOL_TYPE_MOUSE:
        return PointerDeviceType::Mouse;
    case AMOTION_EVENT_TOOL_TYPE_STYLUS:
    case AMOTION_EVENT_TOOL_TYPE_ERASER:
        return PointerDeviceType::Pen;
    default:
        return PointerDeviceType::Touch;
    }
}

std::uint32_t TranslateButtonState(int buttonState) noexcept
{
    std::uint32_t buttons = 0;
    if ((buttonState & AMOTION_EVENT_BUTTON_PRIMARY) != 0) {
        buttons |= 0x01u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_SECONDARY) != 0) {
        buttons |= 0x02u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_TERTIARY) != 0) {
        buttons |= 0x04u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_BACK) != 0) {
        buttons |= 0x08u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_FORWARD) != 0) {
        buttons |= 0x10u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_STYLUS_PRIMARY) != 0) {
        buttons |= 0x20u;
    }
    if ((buttonState & AMOTION_EVENT_BUTTON_STYLUS_SECONDARY) != 0) {
        buttons |= 0x40u;
    }
    return buttons;
}

void SubmitPointer(AInputEvent* event, std::size_t pointerIndex,
                   PointerPhase phase, std::uint32_t buttons,
                   std::uint32_t changedButton, double wheelDelta = 0.0)
{
    const auto pointerId = static_cast<std::uint32_t>(
        AMotionEvent_getPointerId(event, pointerIndex));
    const auto deviceType = GetPointerDeviceType(event, pointerIndex);
    const double x = AMotionEvent_getX(event, pointerIndex);
    const double y = AMotionEvent_getY(event, pointerIndex);
    const float pressure = AMotionEvent_getPressure(event, pointerIndex);

    if (phase != PointerPhase::Move) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android pointer dispatch: id=%u device=%u phase=%u x=%.1f y=%.1f buttons=0x%x changed=0x%x",
            pointerId, static_cast<unsigned>(deviceType),
            static_cast<unsigned>(phase), x, y, buttons, changedButton);
    }

    NeoWPF::Input::InputManager::GetInstance().SubmitPointerEvent(
        PointerEventArgs(pointerId, deviceType, phase, x, y,
                         buttons, changedButton, pressure, wheelDelta));
}

}

int AWindowing::HandlePointerInputEvent(AInputEvent* inputEvent) noexcept
{
    if (m_windows.empty()) {
        return 0;
    }

    const std::size_t pointerCount = AMotionEvent_getPointerCount(inputEvent);
    if (pointerCount == 0) {
        return 0;
    }

    NeoWPF::Core::SetInputRoot(m_windows.front().get());
    const int action = AMotionEvent_getAction(inputEvent);
    const int maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    const std::size_t actionIndex = static_cast<std::size_t>(
        (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
        AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
    if (actionIndex >= pointerCount) {
        return 0;
    }

    switch (maskedAction) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN: {
        const auto deviceType = GetPointerDeviceType(inputEvent, actionIndex);
        const std::uint32_t changedButton = PrimaryButton;
        std::uint32_t buttons = TranslateButtonState(
            AMotionEvent_getButtonState(inputEvent));
        if (deviceType == PointerDeviceType::Touch || buttons == 0) {
            buttons |= PrimaryButton;
        }
        SubmitPointer(inputEvent, actionIndex, PointerPhase::Enter,
                      buttons, changedButton);
        SubmitPointer(inputEvent, actionIndex, PointerPhase::Down,
                      buttons, changedButton);
        return 1;
    }
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP: {
        const auto deviceType = GetPointerDeviceType(inputEvent, actionIndex);
        const std::uint32_t changedButton = PrimaryButton;
        std::uint32_t buttons = TranslateButtonState(
            AMotionEvent_getButtonState(inputEvent));
        if (deviceType == PointerDeviceType::Touch) {
            buttons = 0;
        } else {
            buttons &= ~changedButton;
        }
        SubmitPointer(inputEvent, actionIndex, PointerPhase::Up,
                      buttons, changedButton);
        if (deviceType == PointerDeviceType::Touch) {
            SubmitPointer(inputEvent, actionIndex, PointerPhase::Leave, 0, 0);
        }
        auto& appState = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        for (const auto& windowSession : appState.windowSessions) {
            if (windowSession.renderSurface.nativeWindowHandle == m_windows.front().get()) {
                const auto state = NeoWPF::Core::GetWindowState(windowSession);
                NeoWPF::Diagnostics::LogInfo(
                    "[state:tap] host=%p tree=%d layout=%d nodes=%zu dirty=%zu tree=%zu layout=%zu failed=%d",
                    windowSession.windowHost.get(), state.treeDirty ? 1 : 0,
                    state.layoutDirty ? 1 : 0, state.nodeCount, state.dirtyCount,
                    state.treeDirtyCount, state.layoutDirtyCount, state.failed ? 1 : 0);
                break;
            }
        }
        return 1;
    }
    case AMOTION_EVENT_ACTION_MOVE:
        for (std::size_t pointerIndex = 0; pointerIndex < pointerCount; ++pointerIndex) {
            const auto deviceType = GetPointerDeviceType(inputEvent, pointerIndex);
            std::uint32_t buttons = TranslateButtonState(
                AMotionEvent_getButtonState(inputEvent));
            if (deviceType == PointerDeviceType::Touch) {
                buttons |= PrimaryButton;
            }
            SubmitPointer(inputEvent, pointerIndex, PointerPhase::Move,
                          buttons, 0);
        }
        return 1;
    case AMOTION_EVENT_ACTION_CANCEL:
        for (std::size_t pointerIndex = 0; pointerIndex < pointerCount; ++pointerIndex) {
            SubmitPointer(inputEvent, pointerIndex, PointerPhase::Cancel, 0, 0);
            SubmitPointer(inputEvent, pointerIndex, PointerPhase::Leave, 0, 0);
        }
        return 1;
    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE:
    case AMOTION_EVENT_ACTION_HOVER_EXIT: {
        const auto deviceType = GetPointerDeviceType(inputEvent, actionIndex);
        if (deviceType == PointerDeviceType::Touch) {
            return 0;
        }
        const auto phase = maskedAction == AMOTION_EVENT_ACTION_HOVER_ENTER
            ? PointerPhase::Enter
            : (maskedAction == AMOTION_EVENT_ACTION_HOVER_EXIT
                ? PointerPhase::Leave : PointerPhase::Move);
        SubmitPointer(inputEvent, actionIndex, phase,
            TranslateButtonState(AMotionEvent_getButtonState(inputEvent)), 0);
        return 1;
    }
    case AMOTION_EVENT_ACTION_SCROLL: {
        const float vertical = AMotionEvent_getAxisValue(
            inputEvent, AMOTION_EVENT_AXIS_VSCROLL, actionIndex);
        const float horizontal = AMotionEvent_getAxisValue(
            inputEvent, AMOTION_EVENT_AXIS_HSCROLL, actionIndex);
        const double wheelDelta = (vertical != 0.0f ? vertical : horizontal) * 120.0;
        if (wheelDelta == 0.0) {
            return 0;
        }
        SubmitPointer(inputEvent, actionIndex, PointerPhase::Wheel,
            TranslateButtonState(AMotionEvent_getButtonState(inputEvent)),
            0, wheelDelta);
        return 1;
    }
    default:
        return 0;
    }
}

}