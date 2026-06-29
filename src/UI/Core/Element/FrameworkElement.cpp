#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI
{
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
