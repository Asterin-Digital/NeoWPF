#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI {

    void Alignment::ApplyAlignment(
        FrameworkElement* child,
        double availableWidth,
        double availableHeight,
        HorizontalAlignment hAlign,
        VerticalAlignment vAlign,
        double& outX,
        double& outY,
        double& outWidth,
        double& outHeight
    ) {
        if (!child) return;

        double desiredWidth = child->GetDesiredWidth();
        double desiredHeight = child->GetDesiredHeight();


        if (hAlign == HorizontalAlignment::Stretch) {
            outWidth = availableWidth;
        }
        else {
            outWidth = desiredWidth;
        }
        if (vAlign == VerticalAlignment::Stretch) {
            outHeight = availableHeight;
        }
        else {
            outHeight = desiredHeight;
        }
        switch (hAlign) {
        case HorizontalAlignment::Left:
            outX = 0;
            break;
        case HorizontalAlignment::Center:
            outX = (availableWidth - outWidth) / 2;
            break;
        case HorizontalAlignment::Right:
            outX = availableWidth - outWidth;
            break;
        case HorizontalAlignment::Stretch:
            outX = 0;
            break;
        }

        switch (vAlign) {
        case VerticalAlignment::Top:
            outY = 0;
            break;
        case VerticalAlignment::Center:
            outY = (availableHeight - outHeight) / 2;
            break;
        case VerticalAlignment::Bottom:
            outY = availableHeight - outHeight;
            break;
        case VerticalAlignment::Stretch:
            outY = 0;
            break;
        }
    }

}
