/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <string_view>

namespace NeoWPF::UI::Markup::Xaml {

std::string DecodeXmlEntities(std::string_view value);

}
