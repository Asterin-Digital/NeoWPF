#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/UIElement.h>
#include <NeoWPF/UI/Core/FrameworkElement.h>
#include <algorithm>

namespace NeoWPF::Input
{
    static InputManager g_inputManager;

    InputManager& InputManager::GetInstance()
    {
        return g_inputManager;
    }

    void InputManager::ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta)
    {
        m_mouseX = x;
        m_mouseY = y;

        if (m_captureElement)
        {
            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            m_captureElement->OnMouseInput(&args);
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
            }
            if (targetElement)
            {
                Core::MouseEventArgs enterArgs(x, y, buttonState, wheelDelta);
                targetElement->OnMouseEnter(&enterArgs);
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
                    SetFocusedElement(targetElement);
                }
                else
                {
                    targetElement->OnMouseUp(x, y, buttonState);
                }
            }

            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            targetElement->OnMouseInput(&args);
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
    }

    NeoWPF::UI::UIElement* InputManager::HitTest(int x, int y)
    {
        if (!m_rootElement)
        {
            return nullptr;
        }

        return HitTestElement(m_rootElement, x, y);
    }

    NeoWPF::UI::UIElement* InputManager::HitTestElement(NeoWPF::UI::UIElement* element, int x, int y)
    {
        if (!element)
        {
            return nullptr;
        }

        auto fw = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element);
        if (fw)
        {
            int absX, absY;
            fw->GetAbsolutePosition(absX, absY);
            int width = fw->GetFinalWidth();
            int height = fw->GetFinalHeight();

            if (x < absX || x >= absX + width || y < absY || y >= absY + height)
            {
                return nullptr;
            }
        }

        NeoWPF::UI::UIElement* deepestHit = element;

        auto children = element->GetVisualChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            NeoWPF::UI::UIElement* child = dynamic_cast<NeoWPF::UI::UIElement*>(*it);
            if (!child) continue;
            NeoWPF::UI::UIElement* hit = HitTestElement(child, x, y);
            if (hit)
            {
                return hit;
            }
        }

        return deepestHit;
    }

    void InputManager::SetFocusedElement(NeoWPF::UI::UIElement* element)
    {
        if (m_focusedElement == element)
        {
            return;
        }

        m_focusedElement = element;
    }

    void InputManager::SetMouseCapture(NeoWPF::UI::UIElement* element)
    {
        m_captureElement = element;
    }
}
