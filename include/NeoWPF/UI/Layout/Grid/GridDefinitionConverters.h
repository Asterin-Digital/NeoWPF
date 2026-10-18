/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/TypeConverter.h>

#include <memory>

namespace NeoWPF::UI::Properties {

std::shared_ptr<TypeConverter> GetGridLengthConverter();
std::shared_ptr<TypeConverter> GetGridDefinitionModeConverter();

}
