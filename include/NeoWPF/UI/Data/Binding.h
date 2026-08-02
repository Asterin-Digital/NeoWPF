#pragma once

#include <memory>
#include <optional>
#include <string>

#include <NeoWPF/UI/Data/BindingBase.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

namespace NeoWPF::UI::Data {

class Binding : public BindingBase {
public:
    Binding(std::string path,
            BindingMode mode = BindingMode::OneWay)
        : BindingBase(mode)
        , m_path(std::move(path))
    {
    }

    const std::string& GetPath() const noexcept {
        return m_path;
    }

    Binding& WithMode(BindingMode mode) noexcept {
        m_mode = mode;
        return *this;
    }

    const std::optional<std::weak_ptr<Properties::DependencyObject>>& GetSourceObject() const noexcept {
        return m_sourceObject;
    }

    Binding& WithSource(const std::shared_ptr<Properties::DependencyObject>& source) noexcept {
        m_sourceObject = source;
        return *this;
    }

    const std::optional<std::weak_ptr<INotifyPropertyChanged>>& GetSourcePoco() const noexcept {
        return m_sourcePoco;
    }

    Binding& WithSource(const std::shared_ptr<INotifyPropertyChanged>& source) noexcept {
        m_sourcePoco = source;
        return *this;
    }

private:
    std::string m_path;
    std::optional<std::weak_ptr<Properties::DependencyObject>> m_sourceObject;
    std::optional<std::weak_ptr<INotifyPropertyChanged>> m_sourcePoco;
};

}
