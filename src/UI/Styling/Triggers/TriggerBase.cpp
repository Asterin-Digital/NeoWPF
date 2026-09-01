#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Styling::Triggers {

void TriggerBase::AddSetter(const Setter& setter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_setters.push_back(setter);
}

void TriggerBase::AddSetter(Setter&& setter) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_setters.push_back(std::move(setter));
}

std::vector<Setter> TriggerBase::GetSetters() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_setters;
}

void TriggerBase::Apply(UIElement* element) const {
    if (!element) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& setter : m_setters) {
        setter.Apply(element);
    }
}

bool TriggerBase::Evaluate(UIElement* element) const {
    return element != nullptr;
}

void TriggerBase::Attach(UIElement* element) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_target = element;
}

void TriggerBase::Detach() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_target = nullptr;
}

UIElement* TriggerBase::GetTarget() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_target;
}

}
