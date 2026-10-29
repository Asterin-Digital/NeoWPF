/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <mutex>
#include <vector>

#include <NeoWPF/UI/Styling/Triggers/Setter.h>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling::Triggers {

/*
    @summary
    Base class for conditions that apply setters to an element.
*/
class TriggerBase {
public:
    virtual ~TriggerBase() = default;

    /*
        @summary
        Adds a setter to the trigger.

        @param setter
        Setter to add.
    */
    void AddSetter(const Setter& setter);
    /*
        @summary
        Adds a setter to the trigger by moving it into the collection.

        @param setter
        Setter to add.
    */
    void AddSetter(Setter&& setter);
    /*
        @summary
        Returns the setters registered with the trigger.
    */
    std::vector<Setter> GetSetters() const;

    /*
        @summary
        Applies the trigger setters to an element.

        @param element
        Element to update.
    */
    void Apply(UIElement* element) const;
    /*
        @summary
        Evaluates whether the trigger is active for an element.

        @param element
        Element to evaluate.

        @internal
    */
    virtual bool Evaluate(UIElement* element) const;

    /*
        @summary
        Attaches the trigger to an element.

        @param element
        Element to observe.
    */
    void Attach(UIElement* element);

    /*
        @summary
        Detaches the trigger from its element.
    */
    void Detach();
    /*
        @summary
        Returns the element currently observed by the trigger.

        @internal
    */
    UIElement* GetTarget() const;

protected:
    mutable std::mutex m_mutex;
    UIElement* m_target = nullptr;
    std::vector<Setter> m_setters;
};

}
