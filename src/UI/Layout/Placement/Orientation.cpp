#include <NeoWPF/UI/Layout/Placement/Orientation.h>

namespace NeoWPF::UI {

namespace {

int GetMainAxisExtent(int width, int height, Orientation orientation)
{
    return orientation == Orientation::Horizontal ? width : height;
}

int GetCrossAxisExtent(int width, int height, Orientation orientation)
{
    return orientation == Orientation::Horizontal ? height : width;
}

}

int GetMajorAxisSize(int width, int height, Orientation orientation)
{
    return GetMainAxisExtent(width, height, orientation);
}

int GetMinorAxisSize(int width, int height, Orientation orientation)
{
    return GetCrossAxisExtent(width, height, orientation);
}

}
