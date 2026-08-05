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
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] Starting HitTest at position (%d, %d)", x, y);

        if (!m_rootElement)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] No root element set");
            return nullptr;
        }

        NeoWPF::UI::UIElement* result = HitTestElement(m_rootElement, x, y, 0);
        if (result)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] HitTest complete: HIT element at (%d, %d)", x, y);
        }
        else
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] HitTest complete: NO HIT at (%d, %d)", x, y);
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
        static thread_local char indentBuffer[256] = {};
        for (int i = 0; i < depth && i < 127; ++i)
        {
            indentBuffer[i] = ' ';
        }
        indentBuffer[depth > 127 ? 127 : depth] = '\0';
        indent = indentBuffer;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s├─ Testing element at depth %d", indent, depth);

        if (!element->IsVisible())
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: PASS (element is not visible)", indent);
            return nullptr;
        }

        int elementWidth = 0;
        int elementHeight = 0;
        auto fw = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element);
        if (fw)
        {
            int absX, absY;
            fw->GetAbsolutePosition(absX, absY);
            elementWidth = fw->GetFinalWidth();
            elementHeight = fw->GetFinalHeight();

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Bounds: (%d, %d) size (%d x %d), Test point: (%d, %d)",
                indent, absX, absY, elementWidth, elementHeight, x, y);

            if (x < absX || x >= absX + elementWidth || y < absY || y >= absY + elementHeight)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: MISS (outside bounds)", indent);
                return nullptr;
            }
        }

        const bool isEnabled = element->IsEnabled();
        const bool isHitTestVisible = element->IsHitTestVisible();
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  IsEnabled: %s, IsHitTestVisible: %s", indent, isEnabled ? "true" : "false", isHitTestVisible ? "true" : "false");

        if (!isEnabled)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: PASS (element is disabled)", indent);
            return nullptr;
        }

        NeoWPF::UI::UIElement* deepestHit = isHitTestVisible ? element : nullptr;
        if (!isHitTestVisible)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: PASS (element is not hit-testable, testing children)", indent);
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
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - OUTSIDE", indent, absX, absY, hitWidth, hitHeight);
                        deepestHit = nullptr;
                    }
                    else
                    {
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Content bounds: (%d, %d) size (%d x %d) - INSIDE", indent, absX, absY, hitWidth, hitHeight);
                    }
                }
            }
        }

        auto children = element->GetVisualChildrenInZOrder();
        if (!children.empty())
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Testing %d children (in reverse order - top-most first)...", indent, (int)children.size());
        }

        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            NeoWPF::UI::UIElement* child = dynamic_cast<NeoWPF::UI::UIElement*>(*it);
            if (!child)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s    Skipping null child", indent);
                continue;
            }

            NeoWPF::UI::UIElement* hit = HitTestElement(child, x, y, depth + 1);
            if (hit)
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  → Hit found in child! Returning up the tree.", indent);
                return hit;
            }
        }

        if (deepestHit)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: HIT (element is hit-testable, no children hit)", indent);
        }
        else
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[HitTest] %s  Result: PASS (element not hit-testable, children miss)", indent);
        }

        return deepestHit;
    }
}
