#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

SolidColorBrush::SolidColorBrush() = default;

SolidColorBrush::SolidColorBrush(Color color)
{
    SetColor(color);
}

Color SolidColorBrush::GetColor() const
{
    return GetValue<Color>(ColorProperty());
}

void SolidColorBrush::SetColor(Color color)
{
    SetValue(ColorProperty(), color);
}

}