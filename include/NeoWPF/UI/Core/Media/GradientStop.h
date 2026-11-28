/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <NeoWPF/Core/Events/Notification.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <cstddef>
#include <functional>

namespace NeoWPF::UI {

/*
    @summary
    Defines a color and offset within a gradient.
*/
class GradientStop : public Properties::DependencyObject {
public:
    static const Properties::DependencyProperty& ColorProperty();
    static const Properties::DependencyProperty& OffsetProperty();

    GradientStop() = default;
    GradientStop(Color color, double offset);

    /*
        @summary
        Gets the stop color.

        @returns
        The current color.
    */
    Color GetColor() const;

    /*
        @summary
        Sets the stop color.

        @param color
        The color to use.
    */
    void SetColor(Color color);

    /*
        @summary
        Gets the stop offset.

        @returns
        The current offset.
    */
    double GetOffset() const;

    /*
        @summary
        Sets the stop offset.

        @param offset
        The offset to use.
    */
    void SetOffset(double offset);

    /*
        @summary
        Registers a handler for changes to this stop.

        @param handler
        The handler to invoke when the stop changes.

        @returns
        The identifier used to remove the handler.
    */
    std::size_t AddChangedHandler(std::function<void()> handler);

    /*
        @summary
        Removes a previously registered change handler.

        @param handlerId
        The identifier returned when the handler was registered.
    */
    void RemoveChangedHandler(std::size_t handlerId);

protected:
    /*
        @internal
    */
    void OnDependencyPropertyChanged(
        const Properties::DependencyProperty& property,
        const Properties::PropertyMetadata& metadata,
        const std::any& oldValue,
        const std::any& newValue,
        Properties::PropertyValueSource source) override;

private:
    NeoWPF::Core::Notification<> m_changed;
};

}
