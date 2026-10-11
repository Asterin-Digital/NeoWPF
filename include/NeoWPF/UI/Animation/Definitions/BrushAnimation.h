/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>
#include <NeoWPF/UI/Core/Media/Brush.h>

namespace NeoWPF::UI::Animation {

/*
    @summary
    Animates between brush values.
*/
class BrushAnimation final : public AnimationTimeline {
public:
    /*
        @summary
        Sets the starting brush.

        @param brush
        Starting brush.
    */
    void SetFrom(std::shared_ptr<NeoWPF::UI::Brush> brush) noexcept { m_from = std::move(brush); }

    /*
        @summary
        Sets the ending brush.

        @param brush
        Ending brush.
    */
    void SetTo(std::shared_ptr<NeoWPF::UI::Brush> brush) noexcept { m_to = std::move(brush); }

    /*
        @summary
        Returns the starting brush.
    */
    std::shared_ptr<NeoWPF::UI::Brush> GetFrom() const noexcept { return m_from; }

    /*
        @summary
        Returns the ending brush.
    */
    std::shared_ptr<NeoWPF::UI::Brush> GetTo() const noexcept { return m_to; }

    /*
        @summary
        Returns the interpolated brush for an animation progress value.

        @internal
    */
    std::any GetCurrentValue(double progress) const override;

private:
    std::shared_ptr<NeoWPF::UI::Brush> m_from;
    std::shared_ptr<NeoWPF::UI::Brush> m_to;
};

}