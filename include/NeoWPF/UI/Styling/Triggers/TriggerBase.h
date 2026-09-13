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
    void AddSetter(Setter&& setter);
    std::vector<Setter> GetSetters() const;

    /*
        @summary
        Applies the trigger setters to an element.

        @param element
        Element to update.
    */
    void Apply(UIElement* element) const;
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
    UIElement* GetTarget() const;

protected:
    mutable std::mutex m_mutex;
    UIElement* m_target = nullptr;
    std::vector<Setter> m_setters;
};

}
