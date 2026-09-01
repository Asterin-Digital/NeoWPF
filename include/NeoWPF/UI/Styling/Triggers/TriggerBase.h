#pragma once

#include <mutex>
#include <vector>

#include <NeoWPF/UI/Styling/Triggers/Setter.h>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling::Triggers {

class TriggerBase {
public:
    virtual ~TriggerBase() = default;

    void AddSetter(const Setter& setter);
    void AddSetter(Setter&& setter);
    std::vector<Setter> GetSetters() const;

    void Apply(UIElement* element) const;
    virtual bool Evaluate(UIElement* element) const;

    void Attach(UIElement* element);
    void Detach();
    UIElement* GetTarget() const;

protected:
    mutable std::mutex m_mutex;
    UIElement* m_target = nullptr;
    std::vector<Setter> m_setters;
};

}
