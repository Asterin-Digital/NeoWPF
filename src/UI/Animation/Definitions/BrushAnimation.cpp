#include <NeoWPF/UI/Animation/Definitions/BrushAnimation.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <algorithm>
#include <cmath>

namespace NeoWPF::UI::Animation {

std::any BrushAnimation::GetCurrentValue(double progress) const
{
    const double easedProgress = std::clamp(ApplyEasing(progress), 0.0, 1.0);
    const auto fromBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(m_from);
    const auto toBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(m_to);
    if (!fromBrush || !toBrush) {
        return std::shared_ptr<NeoWPF::UI::Brush>(easedProgress < 1.0 ? m_from : m_to);
    }

    const auto fromArgb = fromBrush->GetArgb();
    const auto toArgb = toBrush->GetArgb();
    std::uint32_t argb = 0;
    for (unsigned int shift = 0; shift < 32; shift += 8) {
        const auto fromChannel = static_cast<double>((fromArgb >> shift) & 0xFFu);
        const auto toChannel = static_cast<double>((toArgb >> shift) & 0xFFu);
        const auto channel = static_cast<std::uint32_t>(std::lround(
            fromChannel + (toChannel - fromChannel) * easedProgress));
        argb |= (channel & 0xFFu) << shift;
    }

    return std::shared_ptr<NeoWPF::UI::Brush>(
        std::make_shared<NeoWPF::UI::SolidColorBrush>(argb));
}

}