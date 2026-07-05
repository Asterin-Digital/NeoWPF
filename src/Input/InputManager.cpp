#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::Input
{
    bool InputManager::s_hitTestLogging = false;

    InputManager& InputManager::GetInstance()
    {
        static InputManager instance;
        return instance;
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
        if (s_hitTestLogging)
        {
            NeoWPF::Diagnostics::LogInfo("[HitTest] Starting HitTest at position (%d, %d)", x, y);
        }

        if (!m_rootElement)
        {
            if (s_hitTestLogging)
            {
                NeoWPF::Diagnostics::LogWarn("[HitTest] No root element set");
            }
            return nullptr;
        }

        NeoWPF::UI::UIElement* result = HitTestElement(m_rootElement, x, y, 0);
        if (s_hitTestLogging)
        {
            if (result)
            {
                NeoWPF::Diagnostics::LogInfo("[HitTest] HitTest complete: HIT element at (%d, %d)", x, y);
            }
            else
            {
                NeoWPF::Diagnostics::LogInfo("[HitTest] HitTest complete: NO HIT at (%d, %d)", x, y);
            }
        }
        return result;
    }

    NeoWPF::UI::UIElement* InputManager::HitTestElement(NeoWPF::UI::UIElement* element, int x, int y, int depth)
    {
        if (!element)
        {
            return nullptr;
        }

        const char* indent = "";
        if (s_hitTestLogging)
        {

            static thread_local char indentBuffer[256] = {};
            for (int i = 0; i < depth && i < 127; ++i)
            {
                indentBuffer[i] = ' ';
            }
            indentBuffer[depth > 127 ? 127 : depth] = '\0';
            indent = indentBuffer;

            NeoWPF::Diagnostics::LogInfo("[HitTest] %s├─ Testing element at depth %d", indent, depth);
        }

        auto fw = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element);
        if (fw)
        {
            int absX, absY;
            fw->GetAbsolutePosition(absX, absY);
            int width = fw->GetFinalWidth();
            int height = fw->GetFinalHeight();

            if (s_hitTestLogging)
            {
                NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Bounds: (%d, %d) size (%d x %d), Test point: (%d, %d)",
                    indent, absX, absY, width, height, x, y);
            }

            if (x < absX || x >= absX + width || y < absY || y >= absY + height)
            {
                if (s_hitTestLogging)
                {
                    NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Result: MISS (outside bounds)", indent);
                }
                return nullptr;
            }
        }

        bool isHitTestVisible = element->IsHitTestVisible();
        if (s_hitTestLogging)
        {
            NeoWPF::Diagnostics::LogInfo("[HitTest] %s  IsHitTestVisible: %s", indent, isHitTestVisible ? "true" : "false");
        }

        NeoWPF::UI::UIElement* deepestHit = isHitTestVisible ? element : nullptr;

        if (deepestHit)
        {
            auto contentAware = dynamic_cast<NeoWPF::UI::IContentAware*>(element);
            if (contentAware)
            {
                NeoWPF::UI::ContentBounds contentBounds = contentAware->GetContentBounds();

                if (contentBounds.width > 0 && contentBounds.height > 0)
                {
                    int absX, absY;
                    fw->GetAbsolutePosition(absX, absY);

                    if (x < absX || x >= absX + contentBounds.width || y < absY || y >= absY + contentBounds.height)
                    {
                        if (s_hitTestLogging)
                        {
                            NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - OUTSIDE", indent, absX, absY, contentBounds.width, contentBounds.height);
                        }
                        deepestHit = nullptr;
                    }
                    else if (s_hitTestLogging)
                    {
                        NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - INSIDE", indent, absX, absY, contentBounds.width, contentBounds.height);
                    }
                }
            }
        }

        auto children = element->GetVisualChildrenInZOrder();
        if (s_hitTestLogging && !children.empty())
        {
            NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Testing %d children (in reverse order - top-most first)...", indent, (int)children.size());
        }

        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            NeoWPF::UI::UIElement* child = dynamic_cast<NeoWPF::UI::UIElement*>(*it);
            if (!child)
            {
                if (s_hitTestLogging)
                {
                    NeoWPF::Diagnostics::LogInfo("[HitTest] %s    Skipping null child", indent);
                }
                continue;
            }

            NeoWPF::UI::UIElement* hit = HitTestElement(child, x, y, depth + 1);
            if (hit)
            {
                if (s_hitTestLogging)
                {
                    NeoWPF::Diagnostics::LogInfo("[HitTest] %s  → Hit found in child! Returning up the tree.", indent);
                }
                return hit;
            }
        }

        if (s_hitTestLogging)
        {
            if (deepestHit)
            {
                NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Result: HIT (element is hit-testable, no children hit)", indent);
            }
            else
            {
                NeoWPF::Diagnostics::LogInfo("[HitTest] %s  Result: PASS (element not hit-testable, children miss)", indent);
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

        if (m_focusedElement)
        {
            m_focusedElement->OnLostFocus();
        }

        m_focusedElement = element;

        if (m_focusedElement)
        {
            m_focusedElement->OnGotFocus();
        }
    }

    void InputManager::SetMouseCapture(NeoWPF::UI::UIElement* element)
    {
        m_captureElement = element;
    }
}
