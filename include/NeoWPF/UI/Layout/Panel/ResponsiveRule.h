/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Events/Notification.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Styling::Triggers {
class Setter;
}

namespace NeoWPF::UI {

/*
    @summary
    Describes dependency-property values to apply within an available-size range.
*/
class ResponsiveRule final : public Properties::DependencyObject {
public:
    ~ResponsiveRule() override;
    static const Properties::DependencyProperty& MinWidthProperty();
    static const Properties::DependencyProperty& MaxWidthProperty();
    static const Properties::DependencyProperty& MinHeightProperty();
    static const Properties::DependencyProperty& MaxHeightProperty();
    static const Properties::DependencyProperty& SettersProperty();

    /*
        @summary
        Returns the inclusive minimum available width in DIPs.
    */
    double GetMinWidth() const;
    /*
        @summary
        Sets the inclusive minimum available width in DIPs.
    */
    void SetMinWidth(double width);
    /*
        @summary
        Returns the exclusive maximum available width in DIPs.
    */
    double GetMaxWidth() const;
    /*
        @summary
        Sets the exclusive maximum available width in DIPs.
    */
    void SetMaxWidth(double width);
    /*
        @summary
        Returns the inclusive minimum available height in DIPs.
    */
    double GetMinHeight() const;
    /*
        @summary
        Sets the inclusive minimum available height in DIPs.
    */
    void SetMinHeight(double height);
    /*
        @summary
        Returns the exclusive maximum available height in DIPs.
    */
    double GetMaxHeight() const;
    /*
        @summary
        Sets the exclusive maximum available height in DIPs.
    */
    void SetMaxHeight(double height);

    /*
        @summary
        Returns the dependency-property setters applied while this rule matches.
    */
    std::vector<std::shared_ptr<Styling::Triggers::Setter>> GetSetters() const;
    /*
        @summary
        Adds a dependency-property setter to this rule.

        @param setter
        Setter to apply when the rule matches.
    */
    void AddSetter(std::shared_ptr<Styling::Triggers::Setter> setter);

    /*
        @internal
    */
    std::size_t AddChangedHandler(std::function<void()> handler);

    /*
        @internal
    */
    void RemoveChangedHandler(std::size_t handlerId);

    /*
        @internal
    */
    bool Matches(double availableWidth, double availableHeight) const;

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
    std::shared_ptr<std::atomic<bool>> m_lifetimeGuard =
        std::make_shared<std::atomic<bool>>(true);
    std::unordered_map<const Styling::Triggers::Setter*, std::size_t>
        m_setterSubscriptions;
    mutable bool m_invalidRangeReported = false;
    NeoWPF::Core::Notification<> m_changed;
};

}
