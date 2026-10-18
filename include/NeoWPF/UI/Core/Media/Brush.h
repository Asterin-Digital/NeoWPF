/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>

namespace NeoWPF::UI {

/*
    @summary
    Abstract base class for objects that paint areas.
*/
class Brush : public Properties::DependencyObject {
public:
    /*
        @summary
        Callback type for brush property changes.
    */
    using ChangedHandler = std::function<void()>;

    virtual ~Brush() = 0;

    static const Properties::DependencyProperty& OpacityProperty();
    static const Properties::DependencyProperty& TransformProperty();
    static const Properties::DependencyProperty& RelativeTransformProperty();

    /*
        @summary
        Gets the opacity applied by this brush.

        @returns
        The brush opacity.
    */
    double GetOpacity() const;

    /*
        @summary
        Sets the opacity applied by this brush.

        @param opacity
        The brush opacity.
    */
    void SetOpacity(double opacity);

    /*
        @summary
        Gets the transform applied to this brush.

        @returns
        The brush transform, or null when no transform is set.
    */
    std::shared_ptr<Core::Transform> GetTransform() const;

    /*
        @summary
        Sets the transform applied to this brush.

        @param transform
        The brush transform, or null to clear it.
    */
    void SetTransform(std::shared_ptr<Core::Transform> transform);

    /*
        @summary
        Gets the relative transform applied to this brush.

        @returns
        The relative brush transform, or null when no transform is set.
    */
    std::shared_ptr<Core::Transform> GetRelativeTransform() const;

    /*
        @summary
        Sets the relative transform applied to this brush.

        @param transform
        The relative brush transform, or null to clear it.
    */
    void SetRelativeTransform(std::shared_ptr<Core::Transform> transform);

    /*
        @summary
        Registers a handler that is called when a brush dependency property changes.

        @param handler
        The callback to register.

        @returns
        A handler identifier, or zero when the callback is empty.
    */
    std::size_t AddChangedHandler(ChangedHandler handler);

    /*
        @summary
        Removes a previously registered change handler.

        @param handlerId
        The identifier returned by AddChangedHandler.
    */
    void RemoveChangedHandler(std::size_t handlerId);

protected:
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;

private:
    std::unordered_map<std::size_t, ChangedHandler> m_changedHandlers;
    std::size_t m_nextChangedHandlerId = 1;
};

}