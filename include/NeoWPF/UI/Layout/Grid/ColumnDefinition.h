/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

/*
    @summary
    Represents the sizing and spacing characteristics of a single column in a Grid.

    @details
    ColumnDefinition is a DependencyObject so it can be created and configured from XAML.
    It holds Width (and related constraints) as dependency properties.
*/
class ColumnDefinition : public Properties::DependencyObject {
public:
    ColumnDefinition() = default;

    /*
        @summary
        Gets the Mode property, which defines the sizing behavior (Auto, Fixed, or Star).
    */
    static const Properties::DependencyProperty& WidthProperty();
    static const Properties::DependencyProperty& ModeProperty();

    /*
        @summary
        Sets the Mode of this column (Auto, Fixed, or Star).

        @param mode
        The sizing mode.
    */
    void SetWidth(Grid::GridLength width);
    Grid::GridLength GetWidth() const;
    void SetMode(int mode);

    /*
        @summary
        Gets the Mode of this column.

        @return
        The sizing mode as an int (cast from SizeMode enum).
    */
    int GetMode() const;
};

}
