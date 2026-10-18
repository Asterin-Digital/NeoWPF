/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>

#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>

namespace NeoWPF::UI::Markup::Xaml {

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> LoadCompiledXaml(
    const NeoWPF::ResourceKey& key,
    XamlNamescope* namescope = nullptr,
    std::string* error = nullptr);

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> LoadCompiledXaml(
    const NeoWPF::ResourceKey& key,
    std::string* error);

}
