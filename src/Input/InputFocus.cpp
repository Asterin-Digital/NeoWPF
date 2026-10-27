#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Diagnostics/Logger.h>
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

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
            "[Input] Focus changed: old=%p new=%p",
            static_cast<void*>(m_focusedElement), static_cast<void*>(element));

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

}
