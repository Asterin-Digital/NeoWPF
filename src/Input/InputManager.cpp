#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <algorithm>

namespace NeoWPF::Input
{

    void InputManager::CollectFocusableElements(NeoWPF::UI::UIElement* root,
                                               std::vector<NeoWPF::UI::UIElement*>& elements) const
    {
        if (!root) {
            return;
        }

        if (root->CanFocus()) {
            elements.push_back(root);
        }

        for (auto* child : root->GetVisualChildren()) {
            if (auto* uiChild = dynamic_cast<NeoWPF::UI::UIElement*>(child)) {
                CollectFocusableElements(uiChild, elements);
            }
        }
    }

    bool InputManager::TryMoveFocus(bool forward)
    {
        if (!m_rootElement) {
            return false;
        }

        std::vector<NeoWPF::UI::UIElement*> focusables;
        CollectFocusableElements(m_rootElement, focusables);

        if (focusables.empty()) {
            return false;
        }

        if (!m_focusedElement) {
            SetFocusedElement(forward ? focusables.front() : focusables.back());
            return true;
        }

        const auto it = std::find(focusables.begin(), focusables.end(), m_focusedElement);
        if (it == focusables.end()) {
            SetFocusedElement(forward ? focusables.front() : focusables.back());
            return true;
        }

        const std::size_t currentIndex = static_cast<std::size_t>(std::distance(focusables.begin(), it));
        const std::size_t nextIndex = forward
            ? (currentIndex + 1) % focusables.size()
            : (currentIndex == 0 ? focusables.size() - 1 : currentIndex - 1);

        if (focusables[currentIndex] == focusables[nextIndex]) {
            return true;
        }

        SetFocusedElement(focusables[nextIndex]);
        return true;
    }

    InputManager& InputManager::GetInstance()
    {
        static InputManager instance;
        return instance;
    }

    void InputManager::SetRootElement(NeoWPF::UI::UIElement* root)
    {
        if (m_rootElement == root)
        {
            return;
        }

        if (m_focusedElement)
        {
            m_focusedElement->OnLostFocus();
            m_focusedElement = nullptr;
        }

        m_captureElement = nullptr;
        m_mouseOverElement = nullptr;
        m_lastHitTestResult = nullptr;
        m_rootElement = root;
    }

    void InputManager::ClickAt(NeoWPF::UI::UIElement* target, int x, int y, std::uint32_t buttonState)
    {
        if (!target) {
            return;
        }

        RouteMouseEvent(target, Core::PreviewMouseDownEvent(), x, y, buttonState, 0);
        target->OnMouseDown(x, y, buttonState);
        RouteMouseEvent(target, Core::MouseDownEvent(), x, y, buttonState, 0);

        RouteMouseEvent(target, Core::PreviewMouseUpEvent(), x, y, buttonState, 0);
        target->OnMouseUp(x, y, buttonState);
        RouteMouseEvent(target, Core::MouseUpEvent(), x, y, buttonState, 0);
    }

    void InputManager::ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta)
    {
        NEO_WPF_ACTION("Input.ProcessMouseInput");

        m_mouseX = x;
        m_mouseY = y;

        if (m_captureElement)
        {
            auto* captured = m_captureElement;
            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            if ((buttonState & 0x01) != (m_prevMouseButtonState & 0x01))
            {
                if ((buttonState & 0x01) != 0)
                {
                    RouteMouseEvent(captured, Core::PreviewMouseDownEvent(), x, y, buttonState, wheelDelta);
                    captured->OnMouseDown(x, y, buttonState);
                    RouteMouseEvent(captured, Core::MouseDownEvent(), x, y, buttonState, wheelDelta);
                }
                else
                {
                    RouteMouseEvent(captured, Core::PreviewMouseUpEvent(), x, y, buttonState, wheelDelta);
                    captured->OnMouseUp(x, y, buttonState);
                    RouteMouseEvent(captured, Core::MouseUpEvent(), x, y, buttonState, wheelDelta);
                }
            }
            RouteMouseEvent(captured, Core::PreviewMouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (wheelDelta != 0)
            {
                RouteMouseEvent(captured, Core::PreviewMouseWheelEvent(), x, y, buttonState, wheelDelta);
            }
            if (captured)
            {
                captured->OnMouseInput(&args);
            }
            RouteMouseEvent(captured, Core::MouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (wheelDelta != 0)
            {
                RouteMouseEvent(captured, Core::MouseWheelEvent(), x, y, buttonState, wheelDelta);
            }
            m_mouseButtonState = buttonState;
            m_prevMouseButtonState = buttonState;
            return;
        }

        NeoWPF::UI::UIElement* targetElement = HitTest(x, y);

        if (targetElement != m_mouseOverElement)
        {
            if (m_mouseOverElement)
            {
                Core::MouseEventArgs leaveArgs(x, y, buttonState, wheelDelta);
                m_mouseOverElement->OnMouseLeave(&leaveArgs);
                RouteMouseEvent(m_mouseOverElement, Core::MouseLeaveEvent(), x, y, buttonState, wheelDelta);
            }
            if (targetElement)
            {
                Core::MouseEventArgs enterArgs(x, y, buttonState, wheelDelta);
                targetElement->OnMouseEnter(&enterArgs);
                RouteMouseEvent(targetElement, Core::MouseEnterEvent(), x, y, buttonState, wheelDelta);
            }
            m_mouseOverElement = targetElement;
        }

        if (targetElement)
        {
            if ((buttonState & 0x01) != (m_prevMouseButtonState & 0x01))
            {
                if ((buttonState & 0x01) != 0)
                {
                    RouteMouseEvent(targetElement, Core::PreviewMouseDownEvent(), x, y, buttonState, wheelDelta);
                    targetElement->OnMouseDown(x, y, buttonState);
                    RouteMouseEvent(targetElement, Core::MouseDownEvent(), x, y, buttonState, wheelDelta);
                    SetFocusedElement(targetElement);
                }
                else
                {
                    RouteMouseEvent(targetElement, Core::PreviewMouseUpEvent(), x, y, buttonState, wheelDelta);
                    targetElement->OnMouseUp(x, y, buttonState);
                    RouteMouseEvent(targetElement, Core::MouseUpEvent(), x, y, buttonState, wheelDelta);
                }
            }

            RouteMouseEvent(targetElement, Core::PreviewMouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (wheelDelta != 0)
            {
                RouteMouseEvent(targetElement, Core::PreviewMouseWheelEvent(), x, y, buttonState, wheelDelta);
            }
            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            targetElement->OnMouseInput(&args);
            RouteMouseEvent(targetElement, Core::MouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (wheelDelta != 0)
            {
                RouteMouseEvent(targetElement, Core::MouseWheelEvent(), x, y, buttonState, wheelDelta);
            }
        }

        m_mouseButtonState = buttonState;
        m_prevMouseButtonState = buttonState;
    }

    void InputManager::ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
    {
        if (keyCode >= 0 && keyCode < static_cast<int>(m_keyStates.size()))
        {
            m_keyStates[keyCode] = isKeyDown;
        }

        if (keyCode == 0x09 && isKeyDown)
        {
            const bool shiftPressed = IsKeyDown(0x10);
            TryMoveFocus(!shiftPressed);
            return;
        }

        if (!m_focusedElement)
        {
            return;
        }

        auto& eventDef = isKeyDown ? Core::PreviewKeyDownEvent() : Core::PreviewKeyUpEvent();
        RouteKeyboardEvent(m_focusedElement, eventDef, keyCode, isKeyDown, charCode, repeatCount);

        Core::KeyboardEventArgs args(keyCode, isKeyDown, charCode, repeatCount);
        m_focusedElement->OnKeyboardInput(&args);

        RouteKeyboardEvent(m_focusedElement, isKeyDown ? Core::KeyDownEvent() : Core::KeyUpEvent(),
                           keyCode, isKeyDown, charCode, repeatCount);
    }

    bool InputManager::IsKeyDown(int keyCode) const
    {
        if (keyCode < 0 || keyCode >= static_cast<int>(m_keyStates.size()))
        {
            return false;
        }
        return m_keyStates[keyCode];
    }

    bool InputManager::IsKeyUp(int keyCode) const
    {
        return !IsKeyDown(keyCode);
    }
}
