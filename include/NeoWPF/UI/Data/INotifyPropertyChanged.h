#pragma once

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace NeoWPF::UI::Data {

using PropertyChangedHandler = std::function<void(const std::string&)>;

class INotifyPropertyChanged {
public:
    virtual ~INotifyPropertyChanged() = default;

    virtual std::size_t AddPropertyChangedHandler(PropertyChangedHandler handler) = 0;
    virtual void RemovePropertyChangedHandler(std::size_t handlerId) = 0;
    virtual std::optional<std::any> GetPropertyValue(const std::string& propertyName) const = 0;
};

class NotifyPropertyChangedBase : public INotifyPropertyChanged {
public:
    std::size_t AddPropertyChangedHandler(PropertyChangedHandler handler) override {
        const std::size_t handlerId = m_nextHandlerId++;
        m_propertyChangedHandlers.emplace(handlerId, std::move(handler));
        return handlerId;
    }

    void RemovePropertyChangedHandler(std::size_t handlerId) override {
        m_propertyChangedHandlers.erase(handlerId);
    }

    void RaisePropertyChanged(const std::string& propertyName) const {
        const auto handlers = m_propertyChangedHandlers;
        for (const auto& [id, handler] : handlers) {
            if (handler) {
                handler(propertyName);
            }
        }
    }

private:
    mutable std::unordered_map<std::size_t, PropertyChangedHandler> m_propertyChangedHandlers;
    mutable std::size_t m_nextHandlerId = 1;
};

}