#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>

namespace NeoWPF::UI::Data {

std::size_t NotifyPropertyChangedBase::AddPropertyChangedHandler(PropertyChangedHandler handler) {
    const std::size_t handlerId = m_nextHandlerId++;
    m_propertyChangedHandlers.emplace(handlerId, std::move(handler));
    return handlerId;
}

void NotifyPropertyChangedBase::RemovePropertyChangedHandler(std::size_t handlerId) {
    m_propertyChangedHandlers.erase(handlerId);
}

std::optional<std::any> NotifyPropertyChangedBase::GetPropertyValue(const std::string& propertyName) const {
    const auto it = m_propertyValues.find(propertyName);
    if (it == m_propertyValues.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool NotifyPropertyChangedBase::SetPropertyValue(const std::string& propertyName, const std::any& value) {
    m_propertyValues[propertyName] = NormalizePropertyValue(value);
    RaisePropertyChanged(propertyName);
    return true;
}

void NotifyPropertyChangedBase::RaisePropertyChanged(const std::string& propertyName) const {
    const auto handlers = m_propertyChangedHandlers;
    for (const auto& [id, handler] : handlers) {
        if (handler) {
            handler(propertyName);
        }
    }
}

}