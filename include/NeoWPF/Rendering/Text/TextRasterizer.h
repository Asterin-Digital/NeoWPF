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

#include <NeoWPF/Rendering/Text/TextTypes.h>

#include <memory>
#include <vector>

namespace NeoWPF::Rendering::Text {

class TextRasterizer {
public:
    virtual ~TextRasterizer() = default;

    virtual TextBitmap Rasterize(const std::vector<GlyphInfo>& glyphs,
                                 const FontRequest& font,
                                 int fontSize) const = 0;
};

class FontDatabase;

std::unique_ptr<TextRasterizer> CreateFreeTypeTextRasterizer(const FontDatabase& fonts);

}
