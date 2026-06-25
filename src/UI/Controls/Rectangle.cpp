#include <NeoWPF/UI/Controls/Rectangle.h>

namespace NeoWPF::UI {

void Rectangle::Measure(int availableWidth, int availableHeight)
{
    int desiredWidth = m_width > 0 ? m_width : availableWidth;
    int desiredHeight = m_height > 0 ? m_height : availableHeight;
    if (desiredWidth < 0) desiredWidth = 0;
    if (desiredHeight < 0) desiredHeight = 0;
    SetDesiredSize(desiredWidth, desiredHeight);
}

void Rectangle::Arrange(int finalWidth, int finalHeight)
{
    if (m_width > 0) {
        finalWidth = m_width;
    }
    if (m_height > 0) {
        finalHeight = m_height;
    }
    FrameworkElement::Arrange(finalWidth, finalHeight);
}

void Rectangle::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    int width = GetFinalWidth();
    int height = GetFinalHeight();
    if (width <= 0 || height <= 0) {
        return;
    }

    commands.DrawRect(
        static_cast<float>(GetLayoutOffsetX()),
        static_cast<float>(GetLayoutOffsetY()),
        static_cast<float>(width),
        static_cast<float>(height),
        m_fillColor);
}

}
