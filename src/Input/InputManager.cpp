#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::Input
{


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

    void InputManager::ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta)
    {
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
                    captured->OnMouseDown(x, y, buttonState);
                }
                else
                {
                    captured->OnMouseUp(x, y, buttonState);
                }
            }
            if (captured)
            {
                captured->OnMouseInput(&args);
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
                    targetElement->OnMouseDown(x, y, buttonState);
                    RouteMouseEvent(targetElement, Core::MouseDownEvent(), x, y, buttonState, wheelDelta);
                    SetFocusedElement(targetElement);
                }
                else
                {
                    targetElement->OnMouseUp(x, y, buttonState);
                    RouteMouseEvent(targetElement, Core::MouseUpEvent(), x, y, buttonState, wheelDelta);
                }
            }

            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            targetElement->OnMouseInput(&args);
            RouteMouseEvent(targetElement, Core::MouseMoveEvent(), x, y, buttonState, wheelDelta);
        }

        m_mouseButtonState = buttonState;
        m_prevMouseButtonState = buttonState;
    }

    void InputManager::ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
    {
        if (!m_focusedElement)
        {
            return;
        }

        Core::KeyboardEventArgs args(keyCode, isKeyDown, charCode, repeatCount);
        m_focusedElement->OnKeyboardInput(&args);
        RouteKeyboardEvent(m_focusedElement, isKeyDown ? Core::KeyDownEvent() : Core::KeyUpEvent(),
                           keyCode, isKeyDown, charCode, repeatCount);
    }
}
