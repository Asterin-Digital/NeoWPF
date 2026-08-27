#include <NeoWPF/Rendering/Text/TextRasterizer.h>
#include <NeoWPF/Rendering/Text/FontDatabase.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>

namespace NeoWPF::Rendering::Text {
namespace {

int PixelsFrom26Dot6(FT_Pos value)
{
    return static_cast<int>((value + 32) / 64);
}

class FreeTypeTextRasterizer final : public TextRasterizer {
public:
    explicit FreeTypeTextRasterizer(const FontDatabase& fonts)
        : m_fonts(fonts)
    {
    }

    TextBitmap Rasterize(const std::vector<GlyphInfo>& glyphs,
                         const FontRequest& request,
                         int fontSize) const override
    {
        TextBitmap bitmap;
        if (glyphs.empty()) {
            return bitmap;
        }

        const auto record = m_fonts.Resolve(request);
        if (!record || !record->data || record->data->empty()) {
            return bitmap;
        }

        FT_Library library = nullptr;
        FT_Face face = nullptr;
        if (FT_Init_FreeType(&library) != 0) {
            return bitmap;
        }

        const auto* bytes = reinterpret_cast<const FT_Byte*>(record->data->data());
        const auto faceResult = FT_New_Memory_Face(
            library, bytes, static_cast<FT_Long>(record->data->size()),
            static_cast<FT_Long>(record->faceIndex), &face);
        if (faceResult != 0 || !face || FT_Set_Pixel_Sizes(face, 0, (std::max)(fontSize, 1)) != 0) {
            if (face) {
                FT_Done_Face(face);
            }
            FT_Done_FreeType(library);
            return bitmap;
        }

        struct PositionedGlyph {
            int width;
            int rows;
            int pitch;
            int bitmapLeft;
            int bitmapTop;
            std::vector<std::uint8_t> coverage;
            int x;
            int y;
        };

        std::vector<PositionedGlyph> positioned;
        positioned.reserve(glyphs.size());
        int penX = 0;
        int minX = 0;
        int maxX = 0;
        int minY = 0;
        int maxY = 0;
        const int baseline = PixelsFrom26Dot6(face->size->metrics.ascender);

        for (const auto& glyph : glyphs) {
            if (FT_Load_Glyph(face, glyph.glyphId, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0) {
                penX += static_cast<int>(std::lround(glyph.xAdvance));
                continue;
            }

            const auto& slot = face->glyph;
            const int glyphX = penX + static_cast<int>(std::lround(glyph.xOffset)) + slot->bitmap_left;
            const int glyphY = baseline - static_cast<int>(std::lround(glyph.yOffset)) - slot->bitmap_top;
            const int pitch = std::abs(slot->bitmap.pitch);
            std::vector<std::uint8_t> coverage(static_cast<std::size_t>(pitch) * slot->bitmap.rows);
            for (unsigned int row = 0; row < slot->bitmap.rows; ++row) {
                const auto* source = slot->bitmap.buffer + row * slot->bitmap.pitch;
                auto* destination = coverage.data() + row * pitch;
                std::copy(source, source + pitch, destination);
            }
            positioned.push_back({
                static_cast<int>(slot->bitmap.width),
                static_cast<int>(slot->bitmap.rows),
                pitch,
                slot->bitmap_left,
                slot->bitmap_top,
                std::move(coverage),
                glyphX,
                glyphY,
            });
            minX = (std::min)(minX, glyphX);
            maxX = (std::max)(maxX, glyphX + static_cast<int>(slot->bitmap.width));
            minY = (std::min)(minY, glyphY);
            maxY = (std::max)(maxY, glyphY + static_cast<int>(slot->bitmap.rows));
            penX += static_cast<int>(std::lround(glyph.xAdvance));
        }

        if (positioned.empty() || maxX <= minX || maxY <= minY) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return bitmap;
        }

        bitmap.width = maxX - minX;
        bitmap.height = maxY - minY;
        bitmap.stride = bitmap.width * 4;
        bitmap.inkTop = minY;
        bitmap.inkHeight = bitmap.height;
        bitmap.rgba.assign(static_cast<std::size_t>(bitmap.stride) * bitmap.height, 0);

        for (const auto& glyph : positioned) {
            for (int row = 0; row < glyph.rows; ++row) {
                for (int column = 0; column < glyph.width; ++column) {
                    const int x = glyph.x - minX + static_cast<int>(column);
                    const int y = glyph.y - minY + static_cast<int>(row);
                    if (x < 0 || x >= bitmap.width || y < 0 || y >= bitmap.height) {
                        continue;
                    }
                    const auto coverage = glyph.coverage[static_cast<std::size_t>(row * glyph.pitch + column)];
                    const auto offset = static_cast<std::size_t>(y * bitmap.stride + x * 4);
                    bitmap.rgba[offset + 0] = 255;
                    bitmap.rgba[offset + 1] = 255;
                    bitmap.rgba[offset + 2] = 255;
                    bitmap.rgba[offset + 3] = (std::max)(bitmap.rgba[offset + 3], coverage);
                }
            }
        }

        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return bitmap;
    }

private:
    const FontDatabase& m_fonts;
};

}

std::unique_ptr<TextRasterizer> CreateFreeTypeTextRasterizer(const FontDatabase& fonts)
{
    return std::make_unique<FreeTypeTextRasterizer>(fonts);
}

}