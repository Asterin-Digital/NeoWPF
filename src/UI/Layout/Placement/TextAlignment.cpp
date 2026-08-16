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

int TextAlignment::GetBlockVerticalOffset(int contentWidth, int contentHeight, int viewportHeight,
                                         TextVerticalAlignment alignment) noexcept
{
    int unusedX = 0;
    int outY = 0;
    ApplyTextAlignment(contentWidth, contentHeight, contentWidth, viewportHeight,
                       TextHorizontalAlignment::Left, alignment, unusedX, outY);
    return outY;
}

int TextAlignment::GetLineHorizontalOffset(int lineWidth, int viewportWidth,
                                         TextHorizontalAlignment alignment) noexcept
{
    int outX = 0;
    int unusedY = 0;
    ApplyTextAlignment(lineWidth, 1, viewportWidth, 1, alignment, TextVerticalAlignment::Top, outX, unusedY);
    return outX;
}

}
