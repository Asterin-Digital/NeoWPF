#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>

namespace NeoWPF::UI {

void TextAlignment::ApplyTextAlignment(
    double contentWidth,
    double contentHeight,
    double availableWidth,
    double availableHeight,
    TextHorizontalAlignment horizontalAlignment,
    TextVerticalAlignment verticalAlignment,
    double& outX,
    double& outY) noexcept
{
    outX = 0.0;
    outY = 0.0;

    if (availableWidth > contentWidth) {
        switch (horizontalAlignment) {
        case TextHorizontalAlignment::Left:
            outX = 0.0;
            break;
        case TextHorizontalAlignment::Center:
            outX = (availableWidth - contentWidth) / 2.0;
            break;
        case TextHorizontalAlignment::Right:
            outX = availableWidth - contentWidth;
            break;
        }
    }

    if (availableHeight > contentHeight) {
        switch (verticalAlignment) {
        case TextVerticalAlignment::Top:
            outY = 0.0;
            break;
        case TextVerticalAlignment::Center:
            outY = (availableHeight - contentHeight) / 2.0;
            break;
        case TextVerticalAlignment::Bottom:
            outY = availableHeight - contentHeight;
            break;
        }
    }
}

double TextAlignment::GetBlockVerticalOffset(double contentWidth, double contentHeight, double viewportHeight,
                                            TextVerticalAlignment alignment) noexcept
{
    double unusedX = 0.0;
    double outY = 0.0;
    ApplyTextAlignment(contentWidth, contentHeight, contentWidth, viewportHeight,
                       TextHorizontalAlignment::Left, alignment, unusedX, outY);
    return outY;
}

double TextAlignment::GetLineHorizontalOffset(double lineWidth, double viewportWidth,
                                            TextHorizontalAlignment alignment) noexcept
{
    double outX = 0.0;
    double unusedY = 0.0;
    ApplyTextAlignment(lineWidth, 1.0, viewportWidth, 1.0, alignment, TextVerticalAlignment::Top, outX, unusedY);
    return outX;
}

void TextAlignment::ApplyTextAlignmentF(
    double contentWidth,
    double contentHeight,
    double availableWidth,
    double availableHeight,
    TextHorizontalAlignment horizontalAlignment,
    TextVerticalAlignment verticalAlignment,
    float& outX,
    float& outY) noexcept
{
    outX = 0.0f;
    outY = 0.0f;

    if (availableWidth > contentWidth) {
        switch (horizontalAlignment) {
        case TextHorizontalAlignment::Left:
            break;
        case TextHorizontalAlignment::Center:
            outX = static_cast<float>((availableWidth - contentWidth) / 2.0);
            break;
        case TextHorizontalAlignment::Right:
            outX = static_cast<float>(availableWidth - contentWidth);
            break;
        }
    }

    if (availableHeight > contentHeight) {
        switch (verticalAlignment) {
        case TextVerticalAlignment::Top:
            break;
        case TextVerticalAlignment::Center:
            outY = static_cast<float>((availableHeight - contentHeight) / 2.0);
            break;
        case TextVerticalAlignment::Bottom:
            outY = static_cast<float>(availableHeight - contentHeight);
            break;
        }
    }
}

float TextAlignment::GetBlockVerticalOffsetF(double contentWidth, double contentHeight, double viewportHeight,
                                            TextVerticalAlignment alignment) noexcept
{
    float unusedX = 0.0f;
    float outY = 0.0f;
    ApplyTextAlignmentF(contentWidth, contentHeight, contentWidth, viewportHeight,
                        TextHorizontalAlignment::Left, alignment, unusedX, outY);
    return outY;
}

float TextAlignment::GetLineHorizontalOffsetF(double lineWidth, double viewportWidth,
                                            TextHorizontalAlignment alignment) noexcept
{
    float outX = 0.0f;
    float unusedY = 0.0f;
    ApplyTextAlignmentF(lineWidth, 1.0, viewportWidth, 1.0, alignment, TextVerticalAlignment::Top, outX, unusedY);
    return outX;
}

}
