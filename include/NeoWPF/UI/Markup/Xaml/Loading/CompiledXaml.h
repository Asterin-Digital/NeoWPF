#pragma once

#include <memory>
#include <string>

#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

namespace NeoWPF::UI::Markup::Xaml {

std::shared_ptr<NeoWPF::UI::Visual> LoadCompiledXaml(const NeoWPF::ResourceKey& key,
                                                     std::string* error = nullptr);

}
