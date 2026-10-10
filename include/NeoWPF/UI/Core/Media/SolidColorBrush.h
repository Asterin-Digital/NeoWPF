/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/UI/Core/Media/Color.h>

namespace NeoWPF::UI {

/*
    @summary
    Paints an area with a single color.
*/
class SolidColorBrush final : public Brush {
public:
    SolidColorBrush();
    explicit SolidColorBrush(Color color);

    static const Properties::DependencyProperty& ColorProperty();

    /*
        @summary
        Gets the color used by this brush.

        @returns
        The solid brush color.
    */
    Color GetColor() const;

    /*
        @summary
        Sets the color used by this brush.

        @param color
        The solid brush color.
    */
    void SetColor(Color color);
};

}