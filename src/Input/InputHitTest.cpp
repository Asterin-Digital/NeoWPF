#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::Input
{
    NeoWPF::UI::UIElement* InputManager::HitTest(int x, int y)
    {
        if (!m_rootElement)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Debug, "[HitTest] No root element set");
            return nullptr;
        }

        NeoWPF::UI::UIElement* result = HitTestElement(m_rootElement, x, y, 0);
        if ((m_lastHitTestResult == nullptr) != (result == nullptr))
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Debug,
                "[HitTest] HitTest boundary changed: %s at (%d, %d)", result ? "HIT element" : "NO HIT", x, y);
        }
        m_lastHitTestResult = result;
        return result;
    }

    NeoWPF::UI::UIElement* InputManager::HitTestVisual(NeoWPF::UI::Visual* visual, int x, int y, int depth)
    {
        if (!visual)
        {
            return nullptr;
        }

        if (auto element = dynamic_cast<NeoWPF::UI::UIElement*>(visual))
        {
            return HitTestElement(element, x, y, depth);
        }

        const char* indent = "";
        static thread_local char indentBuffer[256] = {};
        for (int i = 0; i < depth && i < 127; ++i)
        {
            indentBuffer[i] = ' ';
        }
        indentBuffer[depth > 127 ? 127 : depth] = '\0';
        indent = indentBuffer;

        auto children = visual->GetVisualChildrenInZOrder();
        if (!children.empty())
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s├─ Testing non-UI Visual at depth %d", indent, depth);
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Testing %d children (in reverse order - top-most first)...", indent, (int)children.size());
        }

        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            if (!*it)
            {
                continue;
            }

            NeoWPF::UI::UIElement* hit = HitTestVisual(*it, x, y, depth + 1);
            if (hit)
            {
                return hit;
            }
        }

        return nullptr;
    }

    NeoWPF::UI::UIElement* InputManager::HitTestElement(NeoWPF::UI::UIElement* element, int x, int y, int depth)
    {
        if (!element)
        {
            return nullptr;
        }

        const char* indent = "";
        static thread_local char indentBuffer[256] = {};
        for (int i = 0; i < depth && i < 127; ++i)
        {
            indentBuffer[i] = ' ';
        }
        indentBuffer[depth > 127 ? 127 : depth] = '\0';
        indent = indentBuffer;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s├─ Testing element at depth %d", indent, depth);

        if (!element->IsVisible())
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: PASS (element is not visible)", indent);
            return nullptr;
        }

        int elementWidth = 0;
        int elementHeight = 0;
        int absX = 0;
        int absY = 0;
        auto fw = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element);
        if (fw)
        {
            fw->GetAbsolutePosition(absX, absY);
            elementWidth = fw->GetFinalWidth();
            elementHeight = fw->GetFinalHeight();

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Bounds: (%d, %d) size (%d x %d), Test point: (%d, %d)",
                indent, absX, absY, elementWidth, elementHeight, x, y);

            if (x < absX || x >= absX + elementWidth || y < absY || y >= absY + elementHeight)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: MISS (outside bounds)", indent);
                return nullptr;
            }
        }

        const bool isEnabled = element->IsEnabled();
        const bool isHitTestVisible = element->IsHitTestVisible();
        const bool hasMouseHandlers =
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseDownEvent()) ||
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseUpEvent()) ||
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseMoveEvent()) ||
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseWheelEvent()) ||
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseEnterEvent()) ||
            element->HasRoutedEventHandler(NeoWPF::Input::Core::MouseLeaveEvent());
        const bool isHitTestTarget = isHitTestVisible && (element->IsInteractable() || hasMouseHandlers);

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Info,
            "[HitTest] %s  Element=%s, IsEnabled: %s, IsHitTestVisible: %s, IsHitTestTarget: %s, Bounds=(%d,%d) size=(%d,%d)",
            indent,
            typeid(*element).name(),
            isEnabled ? "true" : "false",
            isHitTestVisible ? "true" : "false",
            isHitTestTarget ? "true" : "false",
            absX,
            absY,
            elementWidth,
            elementHeight);

        if (!isEnabled)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: PASS (element is disabled)", indent);
            return nullptr;
        }

        NeoWPF::UI::UIElement* deepestHit = isHitTestTarget ? element : nullptr;
        if (!isHitTestTarget)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: PASS (element is not an interactive hit-test target, testing children)", indent);
        }

        if (deepestHit)
        {
            auto contentAware = dynamic_cast<NeoWPF::UI::IContentAware*>(element);
            if (contentAware)
            {
                NeoWPF::UI::ContentBounds contentBounds = contentAware->GetContentBounds();

                if (contentBounds.width > 0 && contentBounds.height > 0 && fw)
                {
                    int absX, absY;
                    fw->GetAbsolutePosition(absX, absY);

                    const int hitWidth = std::max(elementWidth, contentBounds.width);
                    const int hitHeight = std::max(elementHeight, contentBounds.height);

                    if (x < absX || x >= absX + hitWidth || y < absY || y >= absY + hitHeight)
                    {
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - OUTSIDE", indent, absX, absY, hitWidth, hitHeight);
                        deepestHit = nullptr;
                    }
                    else
                    {
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - INSIDE", indent, absX, absY, hitWidth, hitHeight);
                    }
                }
            }
        }

        auto children = element->GetVisualChildrenInZOrder();
        if (!children.empty())
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Testing %d children (in reverse order - top-most first)...", indent, (int)children.size());
        }

        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            if (!*it)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s    Skipping null child", indent);
                continue;
            }

            NeoWPF::UI::UIElement* hit = HitTestVisual(*it, x, y, depth + 1);
            if (hit)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  → Hit found in child! Returning up the tree.", indent);
                return hit;
            }
        }

        if (deepestHit)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: HIT (element is hit-testable, no children hit)", indent);
        }
        else
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::HitTest, NeoWPF::Diagnostics::Level::Trace, "[HitTest] %s  Result: PASS (element not hit-testable, children miss)", indent);
        }

        return deepestHit;
    }
}
