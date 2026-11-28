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
#include <NeoWPF/UI/Layout/LayoutEnvironment.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <functional>
#include <memory>
#include <vector>

namespace NeoWPF::UI::Styling::Triggers {
class Setter;
}

namespace NeoWPF::UI::Properties {
class TypeConverter;
std::shared_ptr<const TypeConverter> GetResponsiveScopeConverter();
std::shared_ptr<const TypeConverter> GetResponsivePointerConverter();
std::shared_ptr<const TypeConverter> GetResponsiveHoverConverter();
std::shared_ptr<const TypeConverter> GetResponsiveKeyboardConverter();
}

namespace NeoWPF::UI {

enum class ResponsiveScope {
    Container,
    Window
};

enum class ResponsivePointer {
    Any,
    Fine,
    Coarse
};

enum class ResponsiveHover {
    Any,
    On,
    Off
};

enum class ResponsiveKeyboard {
    Any,
    Shown,
    Hidden
};

/*
    @summary
    Defines size conditions and setters applied by a responsive layout host.
*/
class ResponsiveRule final : public Properties::DependencyObject {
public:
    static const Properties::DependencyProperty& MinWidthProperty();
    static const Properties::DependencyProperty& MaxWidthProperty();
    static const Properties::DependencyProperty& MinHeightProperty();
    static const Properties::DependencyProperty& MaxHeightProperty();
    static const Properties::DependencyProperty& MinAspectRatioProperty();
    static const Properties::DependencyProperty& MaxAspectRatioProperty();
    static const Properties::DependencyProperty& ScopeProperty();
    static const Properties::DependencyProperty& PointerProperty();
    static const Properties::DependencyProperty& HoverProperty();
    static const Properties::DependencyProperty& KeyboardProperty();
    static const Properties::DependencyProperty& SettersProperty();

    /*
        @summary
        Gets the minimum width required for this rule to match.
    */
    double GetMinWidth() const;

    /*
        @summary
        Sets the minimum width required for this rule to match.

        @param width
        The minimum width.
    */
    void SetMinWidth(double width);

    /*
        @summary
        Gets the maximum width below which this rule can match.
    */
    double GetMaxWidth() const;

    /*
        @summary
        Sets the maximum width below which this rule can match.

        @param width
        The maximum width.
    */
    void SetMaxWidth(double width);

    /*
        @summary
        Gets the minimum height required for this rule to match.
    */
    double GetMinHeight() const;

    /*
        @summary
        Sets the minimum height required for this rule to match.

        @param height
        The minimum height.
    */
    void SetMinHeight(double height);

    /*
        @summary
        Gets the maximum height below which this rule can match.
    */
    double GetMaxHeight() const;

    /*
        @summary
        Sets the maximum height below which this rule can match.

        @param height
        The maximum height.
    */
    void SetMaxHeight(double height);

    /*
        @summary
        Gets the minimum aspect ratio required for this rule to match.
    */
    double GetMinAspectRatio() const;

    /*
        @summary
        Sets the minimum aspect ratio required for this rule to match.

        @param ratio
        The minimum aspect ratio.
    */
    void SetMinAspectRatio(double ratio);

    /*
        @summary
        Gets the maximum aspect ratio below which this rule can match.
    */
    double GetMaxAspectRatio() const;

    /*
        @summary
        Sets the maximum aspect ratio below which this rule can match.

        @param ratio
        The maximum aspect ratio.
    */
    void SetMaxAspectRatio(double ratio);

    /*
        @summary
        Gets the layout scope used to evaluate this rule.
    */
    ResponsiveScope GetScope() const;

    /*
        @summary
        Sets the layout scope used to evaluate this rule.

        @param scope
        The layout scope.
    */
    void SetScope(ResponsiveScope scope);

    /*
        @summary
        Gets the pointer precision required for this rule to match.
    */
    ResponsivePointer GetPointer() const;

    /*
        @summary
        Sets the pointer precision required for this rule to match.

        @param pointer
        The pointer precision.
    */
    void SetPointer(ResponsivePointer pointer);

    /*
        @summary
        Gets the hover state required for this rule to match.
    */
    ResponsiveHover GetHover() const;

    /*
        @summary
        Sets the hover state required for this rule to match.

        @param hover
        The required hover state.
    */
    void SetHover(ResponsiveHover hover);

    /*
        @summary
        Gets the keyboard visibility required for this rule to match.
    */
    ResponsiveKeyboard GetKeyboard() const;

    /*
        @summary
        Sets the keyboard visibility required for this rule to match.

        @param keyboard
        The required keyboard visibility.
    */
    void SetKeyboard(ResponsiveKeyboard keyboard);

    /*
        @summary
        Gets the setters applied when this rule matches.

        @returns
        The setters configured for this rule.
    */
    std::vector<std::shared_ptr<Styling::Triggers::Setter>> GetSetters() const;

    /*
        @summary
        Adds a setter to apply when this rule matches.

        @param setter
        The setter to add.
    */
    void AddSetter(std::shared_ptr<Styling::Triggers::Setter> setter);

    /*
        @summary
        Checks whether the size satisfies this rule's conditions.

        @param width
        The width to evaluate.

        @param height
        The height to evaluate.

        @returns
        True when all size conditions match.
    */
    bool Matches(double width, double height) const;

    /*
        @summary
        Checks whether the size and input environment satisfy this rule's conditions.

        @param width
        The width to evaluate.

        @param height
        The height to evaluate.

        @param pointer
        The pointer precision to evaluate.

        @param hover
        Whether hover is available.

        @param keyboardShown
        Whether the on-screen keyboard is shown.

        @returns
        True when all size and input conditions match.
    */
    bool Matches(double width,
                 double height,
                 NeoWPF::UI::Layout::PointerPrecision pointer,
                 bool hover,
                 bool keyboardShown) const;

    /*
        @summary
        Subscribes to changes in this rule.

        @param handler
        The callback invoked when the rule changes.

        @returns
        The subscription identifier.

        @internal
    */
    std::size_t AddChangedHandler(std::function<void()> handler);

    /*
        @summary
        Removes a change subscription from this rule.

        @param handlerId
        The subscription identifier returned by AddChangedHandler.

        @internal
    */
    void RemoveChangedHandler(std::size_t handlerId);

private:
    /*
        @summary
        Publishes changes to responsive rule properties.

        @internal
    */
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;

    std::shared_ptr<NeoWPF::Core::Notification<>> m_changed =
        std::make_shared<NeoWPF::Core::Notification<>>();
};

}
