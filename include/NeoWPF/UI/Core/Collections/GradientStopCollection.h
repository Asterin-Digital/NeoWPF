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

#include <NeoWPF/UI/Core/Media/GradientStop.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace NeoWPF::UI {

/*
    @summary
    Provides a constant iterator over gradient stops.
*/
class GradientStopCollection final {
public:
    /*
        @summary
        Constant iterator type for the collection.
    */
    using const_iterator = std::vector<std::shared_ptr<GradientStop>>::const_iterator;

    GradientStopCollection() = default;
    GradientStopCollection(const GradientStopCollection&) = delete;

    /*
        @summary
        Disables copy assignment for the collection.

        @param other
        The collection to assign from.
    */
    GradientStopCollection& operator=(const GradientStopCollection& other) = delete;
    ~GradientStopCollection();

    /*
        @summary
        Gets the number of stops.

        @returns
        The number of stops in the collection.
    */
    std::size_t size() const noexcept;

    /*
        @summary
        Checks whether the collection has no stops.

        @returns
        True when the collection is empty.
    */
    bool empty() const noexcept;

    /*
        @summary
        Gets an iterator to the first stop.

        @returns
        An iterator to the first stop.
    */
    const_iterator begin() const noexcept;

    /*
        @summary
        Gets an iterator past the last stop.

        @returns
        An iterator past the last stop.
    */
    const_iterator end() const noexcept;

    /*
        @summary
        Adds a stop to the collection.

        @param stop
        The stop to add.
    */
    void Add(std::shared_ptr<GradientStop> stop);

    /*
        @summary
        Removes a stop from the collection.

        @param stop
        The stop to remove.

        @returns
        True if the stop was present and removed.
    */
    bool Remove(const std::shared_ptr<GradientStop>& stop);

    /*
        @summary
        Removes all stops from the collection.
    */
    void Clear();

    /*
        @summary
        Registers a handler for collection or stop changes.

        @param handler
        The handler to invoke when the collection changes.

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

private:
    void UnsubscribeFromStops() noexcept;

    std::vector<std::shared_ptr<GradientStop>> m_stops;
    std::vector<std::size_t> m_stopHandlerIds;
    NeoWPF::Core::Notification<> m_changed;
};

}
