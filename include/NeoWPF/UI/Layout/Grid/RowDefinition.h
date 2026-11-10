/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <any>
#include <vector>

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

    /*
        @summary
        Returns the height assigned to this row.
    */
    Grid::GridLength GetHeight() const;

    /*
        @summary
        Sets the sizing mode for this row.

        @param mode
        The sizing mode to apply.
    */
    void SetMode(int mode);

    /*
        @summary
        Gets the Mode of this row.

        @return
        The sizing mode as an int (cast from SizeMode enum).
    */
    int GetMode() const;

protected:
    /*
        @summary
        Notifies owning Grids when the row height changes.

        @internal
    */
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;

private:
    friend class Grid;

    /*
        @summary
        Registers a Grid that uses this definition.

        @internal
    */
    void AddOwner(const std::shared_ptr<GridDefinitionOwner>& owner);

    /*
        @summary
        Removes a Grid that no longer uses this definition.

        @internal
    */
    void RemoveOwner(const std::shared_ptr<GridDefinitionOwner>& owner) noexcept;

    std::vector<std::shared_ptr<GridDefinitionOwner>> m_owners;
};

}
