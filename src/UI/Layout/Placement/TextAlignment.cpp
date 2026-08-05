#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>

namespace NeoWPF::UI {

void TextAlignment::ApplyTextAlignment(
    int contentWidth,
    int contentHeight,
    int availableWidth,
    int availableHeight,
    TextHorizontalAlignment horizontalAlignment,
    TextVerticalAlignment verticalAlignment,
    int& outX,
    int& outY) noexcept
{
    outX = 0;
    outY = 0;

    if (availableWidth > contentWidth) {
        switch (horizontalAlignment) {
        case TextHorizontalAlignment::Left:
            outX = 0;
            break;
        case TextHorizontalAlignment::Center:
            outX = (availableWidth - contentWidth) / 2;
            break;
        case TextHorizontalAlignment::Right:
            outX = availableWidth - contentWidth;
            break;
        }
    }

    if (availableHeight > contentHeight) {
        switch (verticalAlignment) {
        case TextVerticalAlignment::Top:
            outY = 0;
            break;
        case TextVerticalAlignment::Center:
            outY = (availableHeight - contentHeight) / 2;
            break;
        case TextVerticalAlignment::Bottom:
            outY = availableHeight - contentHeight;
            break;
        }
    }
}

}
