#pragma once

#include <memory>
#include <string>

#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>

namespace NeoWPF::UI::Markup::Xaml {

std::shared_ptr<NeoWPF::UI::Visual> LoadCompiledXaml(const NeoWPF::ResourceKey& key,
                                                     XamlNamescope* namescope = nullptr,
                                                     std::string* error = nullptr);

std::shared_ptr<NeoWPF::UI::Visual> LoadCompiledXaml(const NeoWPF::ResourceKey& key,
                                                     std::string* error);

}
