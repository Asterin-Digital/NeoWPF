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
