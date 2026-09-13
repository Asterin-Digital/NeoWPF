/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace NeoWPF::UI::Markup::Xaml {

class XamlNamescope {
public:
    bool Register(const std::string& name,
                  const std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>& object)
    {
        if (name.empty() || !object || m_objects.contains(name)) {
            return false;
        }

        m_objects.emplace(name, object);
        return true;
    }

    std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> Find(const std::string& name) const
    {
        const auto iterator = m_objects.find(name);
        return iterator == m_objects.end() ? nullptr : iterator->second;
    }

    template<typename T>
    std::shared_ptr<T> Find(const std::string& name) const
    {
        return std::dynamic_pointer_cast<T>(Find(name));
    }

private:
    std::unordered_map<std::string, std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>> m_objects;
};

}
