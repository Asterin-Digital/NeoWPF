#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <algorithm>

#ifdef max
#undef max
#endif

namespace NeoWPF {
namespace UI
{
    int FrameworkElement::ResolveWidth(int availableWidth, int desiredWidth) const
    {
        if (m_widthMode == LayoutSizeMode::Explicit) {
            return (std::max)(0, m_explicitWidth);
        }
        if (m_widthMode == LayoutSizeMode::Stretch) {
            return (std::max)(0, availableWidth);
        }
        return (std::max)(0, desiredWidth);
    }

    int FrameworkElement::ResolveHeight(int availableHeight, int desiredHeight) const
    {
        if (m_heightMode == LayoutSizeMode::Explicit) {
            return (std::max)(0, m_explicitHeight);
        }
        if (m_heightMode == LayoutSizeMode::Stretch) {
            return (std::max)(0, availableHeight);
        }
        return (std::max)(0, desiredHeight);
    }

    void FrameworkElement::GetAbsolutePosition(int& outX, int& outY) const noexcept
    {
        outX = m_offsetX;
        outY = m_offsetY;

        Visual* parent = GetParent();
        while (parent)
        {
            auto fw = dynamic_cast<FrameworkElement*>(parent);
            if (fw)
            {
                outX += fw->GetLayoutOffsetX();
                outY += fw->GetLayoutOffsetY();
            }
            parent = parent->GetParent();
        }
    }
}
}
