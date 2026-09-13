/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <any>
#include <string>

namespace NeoWPF::UI::Styling::Triggers::Detail {

inline bool AnyEquals(const NeoWPF::UI::Properties::DependencyObject& owner,
                      const NeoWPF::UI::Properties::DependencyProperty& property,
                      const std::any& expectedValue)
{
    return NeoWPF::UI::Properties::ValuesEqual(owner, property, expectedValue);
}

}