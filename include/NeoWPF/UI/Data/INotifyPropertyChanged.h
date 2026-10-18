/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <cwchar>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

namespace NeoWPF::UI::Data {

using PropertyChangedHandler = std::function<void(const std::string&)>;

class INotifyPropertyChanged {
public:
    virtual ~INotifyPropertyChanged() = default;

    virtual std::size_t AddPropertyChangedHandler(PropertyChangedHandler handler) = 0;
    virtual void RemovePropertyChangedHandler(std::size_t handlerId) = 0;
    virtual std::optional<std::any> GetPropertyValue(const std::string& propertyName) const = 0;
    virtual bool SetPropertyValue(const std::string& propertyName, const std::any& value) = 0;
};

inline std::any NormalizePropertyValue(const std::any& value)
{
    if (!value.has_value()) {
        return value;
    }

    if (value.type() == typeid(const wchar_t*)) {
        return std::any(std::wstring(std::any_cast<const wchar_t*>(value)));
    }

    if (value.type() == typeid(const char*)) {
        return std::any(std::string(std::any_cast<const char*>(value)));
    }

    return value;
}

inline std::any NormalizePropertyValue(const wchar_t* value)
{
    return std::any(std::wstring(value ? value : L""));
}

inline std::any NormalizePropertyValue(const char* value)
{
    return std::any(std::string(value ? value : ""));
}

template<typename T>
std::any NormalizePropertyValue(const T& value)
{
    return std::any(value);
}

template<std::size_t N>
std::any NormalizePropertyValue(const wchar_t (&value)[N])
{
    return std::any(std::wstring(value));
}

template<std::size_t N>
std::any NormalizePropertyValue(const char (&value)[N])
{
    return std::any(std::string(value));
}

class NotifyPropertyChangedBase : public INotifyPropertyChanged {
public:
    std::size_t AddPropertyChangedHandler(PropertyChangedHandler handler) override;
    void RemovePropertyChangedHandler(std::size_t handlerId) override;
    std::optional<std::any> GetPropertyValue(const std::string& propertyName) const override;
    bool SetPropertyValue(const std::string& propertyName, const std::any& value) override;

    template<typename T>
    void SetProperty(const std::string& propertyName, const T& value) {
        m_propertyValues[propertyName] = NormalizePropertyValue(value);
        RaisePropertyChanged(propertyName);
    }

    template<typename T>
    void SetProperty(const std::string& propertyName, const std::shared_ptr<T>& value) {
        if constexpr (std::is_base_of_v<INotifyPropertyChanged, T>) {
            m_propertyValues[propertyName] = std::any(std::shared_ptr<INotifyPropertyChanged>(value));
        } else {
            m_propertyValues[propertyName] = std::any(value);
        }
        RaisePropertyChanged(propertyName);
    }

    template<typename T>
    void SetProperty(const std::wstring& propertyName, const T& value) {
        std::string narrowName;
        narrowName.reserve(propertyName.size());
        for (const wchar_t ch : propertyName) {
            narrowName.push_back(static_cast<char>(ch));
        }
        SetProperty(narrowName, value);
    }

    template<typename T>
    void SetProperty(const std::wstring& propertyName, const std::shared_ptr<T>& value) {
        std::string narrowName;
        narrowName.reserve(propertyName.size());
        for (const wchar_t ch : propertyName) {
            narrowName.push_back(static_cast<char>(ch));
        }
        SetProperty(narrowName, value);
    }

    template<typename T>
    std::optional<T> GetProperty(const std::string& propertyName) const {
        const auto value = GetPropertyValue(propertyName);
        if (!value.has_value()) {
            return std::nullopt;
        }

        try {
            return std::any_cast<T>(*value);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    template<typename T>
    std::optional<T> GetProperty(const std::wstring& propertyName) const {
        std::string narrowName;
        narrowName.reserve(propertyName.size());
        for (const wchar_t ch : propertyName) {
            narrowName.push_back(static_cast<char>(ch));
        }
        return GetProperty<T>(narrowName);
    }

    void RaisePropertyChanged(const std::string& propertyName) const;

protected:
    mutable std::unordered_map<std::size_t, PropertyChangedHandler> m_propertyChangedHandlers;
    mutable std::unordered_map<std::string, std::any> m_propertyValues;
    mutable std::size_t m_nextHandlerId = 1;
};

class PropertyBagNotify : public NotifyPropertyChangedBase {
public:
    using NotifyPropertyChangedBase::GetProperty;
    using NotifyPropertyChangedBase::GetPropertyValue;
    using NotifyPropertyChangedBase::SetProperty;
};

}