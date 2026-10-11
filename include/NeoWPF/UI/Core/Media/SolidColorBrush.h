/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Media/Brush.h>

#include <cstdint>

namespace NeoWPF::UI {

/*
    @summary
    Paints an area with a single color.
*/
class SolidColorBrush final : public Brush {
public:
    SolidColorBrush();
    explicit SolidColorBrush(std::uint32_t argb);

    static const Properties::DependencyProperty& ArgbProperty();

    /*
        @summary
        Gets the ARGB value used by this brush.

        @returns
        The packed ARGB value.
    */
        std::uint32_t GetArgb() const;

    /*
        @summary
        Sets the ARGB value used by this brush.

            @param argb
            The packed ARGB value.
    */
        void SetArgb(std::uint32_t argb);
};

}