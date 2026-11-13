#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Rendering/Renderer.h>

#include <algorithm>

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

NeoWPF::Rendering::Paint SolidColorBrush::ResolvePaint(
    NeoWPF::Rendering::CommandList&,
    double,
    double,
    double,
    double) const
{
    const std::uint32_t argb = GetArgb();
    const auto opacity = (std::clamp)(GetOpacity(), 0.0, 1.0);
    const auto alpha = static_cast<std::uint32_t>(((argb >> 24) & 0xFFu) * opacity);
    return NeoWPF::Rendering::Paint::Solid((argb & 0x00FFFFFFu) | (alpha << 24));
}

}