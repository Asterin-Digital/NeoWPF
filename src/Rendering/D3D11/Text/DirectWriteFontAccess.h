#pragma once

#include <dwrite.h>
#include <string>

namespace NeoWPF::Rendering::Text {

IDWriteFontFace* CreateRegisteredFontFace(const std::wstring& familyName);
IDWriteFontCollection* GetCustomFontCollection();

}
