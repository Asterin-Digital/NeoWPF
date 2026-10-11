#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

SolidColorBrush::SolidColorBrush() = default;

SolidColorBrush::SolidColorBrush(std::uint32_t argb)
{
    SetArgb(argb);
}

std::uint32_t SolidColorBrush::GetArgb() const
{
    return GetValue<std::uint32_t>(ArgbProperty());
}

void SolidColorBrush::SetArgb(std::uint32_t argb)
{
    SetValue(ArgbProperty(), argb);
}

}