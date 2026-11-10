/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>

#include <NeoWPF/Etc/Result.h>
#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>

namespace NeoWPF::UI::Markup::Xaml {

/*
    @summary
    Loads a compiled XAML resource into a dependency object tree.

    @param key
    Key identifying the compiled XAML resource.

    @param namescope
    Optional namescope populated while loading.

    @returns
    The loaded root object, or the failure reason.
*/
NeoWPF::Result<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>> LoadCompiledXaml(
    const NeoWPF::ResourceKey& key,
    XamlNamescope* namescope = nullptr);

}
