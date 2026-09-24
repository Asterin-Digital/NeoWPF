/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

/*
    @summary
    Represents the sizing and spacing characteristics of a single row in a Grid.

    @details
    RowDefinition is a DependencyObject so it can be created and configured from XAML.
    It holds Height (and related constraints) as dependency properties.
*/
class RowDefinition : public Properties::DependencyObject {
public:
    RowDefinition() = default;

    /*
        @summary
        Gets the Mode property, which defines the sizing behavior (Auto, Fixed, or Star).
    */
    static const Properties::DependencyProperty& HeightProperty();
    static const Properties::DependencyProperty& ModeProperty();

    /*
        @summary
        Sets the Mode of this row (Auto, Fixed, or Star).

        @param mode
        The sizing mode.
    */
    void SetHeight(Grid::GridLength height);
    Grid::GridLength GetHeight() const;
    void SetMode(int mode);

    /*
        @summary
        Gets the Mode of this row.

        @return
        The sizing mode as an int (cast from SizeMode enum).
    */
    int GetMode() const;
};

}
