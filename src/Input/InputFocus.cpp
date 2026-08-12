#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::Input
{
    void FocusElement(NeoWPF::UI::UIElement* element)
    {
        if (!element) {
            return;
        }

        InputManager::GetInstance().SetFocusedElement(element);
    }

    void InputManager::SetFocusedElement(NeoWPF::UI::UIElement* element)
    {
        if (m_focusedElement == element)
        {
            return;
        }

        if (m_focusedElement)
        {
            m_focusedElement->SetFocused(false);
            m_focusedElement->OnLostFocus();
        }

        m_focusedElement = element;

        if (m_focusedElement)
        {
            m_focusedElement->SetFocused(true);
            m_focusedElement->OnGotFocus();
        }
    }

    void InputManager::SetMouseCapture(NeoWPF::UI::UIElement* element)
    {
        m_captureElement = element;
    }
}
