#pragma once

#include <NeoWPF/UI/Properties/TypeConverter.h>

#include <memory>

namespace NeoWPF::UI::Properties {

std::shared_ptr<TypeConverter> GetGridLengthConverter();
std::shared_ptr<TypeConverter> GetGridDefinitionModeConverter();

}
