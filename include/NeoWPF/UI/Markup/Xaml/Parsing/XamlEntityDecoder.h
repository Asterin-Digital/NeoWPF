/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <string>
#include <string_view>

namespace NeoWPF::UI::Markup::Xaml {

std::string DecodeXmlEntities(std::string_view value);

}
