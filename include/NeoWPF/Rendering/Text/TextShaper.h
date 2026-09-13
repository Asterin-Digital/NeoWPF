/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Text/FontDatabase.h>
#include <NeoWPF/Rendering/Text/TextTypes.h>

#include <memory>
#include <string_view>
#include <vector>

namespace NeoWPF::Rendering::Text {

class TextShaper {
public:
    virtual ~TextShaper() = default;

    virtual std::vector<GlyphInfo> Shape(std::string_view utf8,
                                         const FontRequest& font,
                                         int fontSize) const = 0;
};

std::unique_ptr<TextShaper> CreateHarfBuzzTextShaper(const FontDatabase& fonts);

}
