#pragma once

#include <memory>
#include <optional>
#include <string>

#include <NeoWPF/UI/Data/BindingBase.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

namespace NeoWPF::UI::Data {
class INotifyPropertyChanged;
}

namespace NeoWPF::UI::Data {

/*
    @summary
    Describes a property path and its binding source.
*/
class Binding : public BindingBase {
public:
    /*
        @summary
        Creates a binding for a property path.

        @param path
        Source property path.

        @param mode
        Binding update mode.
    */
    Binding(std::string path,
            BindingMode mode = BindingMode::OneWay)
        : BindingBase(mode)
        , m_path(std::move(path))
    {
    }

    const std::string& GetPath() const noexcept {
        return m_path;
    }

    /*
        @summary
        Sets the binding update mode.

        @param mode
        Binding update mode.
    */
    Binding& WithMode(BindingMode mode) noexcept {
        m_mode = mode;
        return *this;
    }

    const std::optional<std::weak_ptr<Properties::DependencyObject>>& GetSourceObject() const noexcept {
        return m_sourceObject;
    }

    /*
        @summary
        Sets a dependency object as the binding source.

        @param source
        Binding source.
    */
    Binding& WithSource(const std::shared_ptr<Properties::DependencyObject>& source) noexcept {
        m_sourceObject = source;
        return *this;
    }

    const std::optional<std::weak_ptr<INotifyPropertyChanged>>& GetSourcePoco() const noexcept {
        return m_sourcePoco;
    }

    /*
        @summary
        Sets a property-notifying object as the binding source.

        @param source
        Binding source.
    */
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
