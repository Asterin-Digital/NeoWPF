#include <NeoWPF/Rendering/Text/TextShaper.h>

#include <hb.h>
#include <hb-ot.h>

#include <algorithm>
#include <vector>

namespace NeoWPF::Rendering::Text {
namespace {

class HarfBuzzTextShaper final : public TextShaper {
public:
    explicit HarfBuzzTextShaper(const FontDatabase& fonts)
        : m_fonts(fonts)
    {
    }

    std::vector<GlyphInfo> Shape(std::string_view utf8, const FontRequest& request,
                                 int fontSize) const override
    {
        if (utf8.empty()) {
            return {};
        }

        const auto record = m_fonts.Resolve(request);
        if (!record || !record->data || record->data->empty()) {
            return {};
        }

        hb_blob_t* blob = hb_blob_create(
            reinterpret_cast<const char*>(record->data->data()),
            static_cast<unsigned int>(record->data->size()),
            HB_MEMORY_MODE_READONLY,
            nullptr,
            nullptr);
        if (!blob) {
            return {};
        }

        hb_face_t* face = hb_face_create(blob, 0);
        hb_blob_destroy(blob);
        if (!face) {
            return {};
        }

        hb_font_t* font = hb_font_create(face);
        hb_face_destroy(face);
        if (!font) {
            return {};
        }

        hb_ot_font_set_funcs(font);
        const int fontScale = (std::max)(fontSize, 1) * 64;
        constexpr float kHarfBuzzUnitScale = 64.0f;
        hb_font_set_scale(font, fontScale, fontScale);
        hb_buffer_t* buffer = hb_buffer_create();
        if (!buffer) {
            hb_font_destroy(font);
            return {};
        }

        hb_buffer_add_utf8(buffer, utf8.data(), static_cast<int>(utf8.size()), 0, -1);
        hb_buffer_guess_segment_properties(buffer);
        hb_shape(font, buffer, nullptr, 0);

        unsigned int glyphCount = 0;
        const hb_glyph_info_t* glyphInfos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
        const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyphCount);

        std::vector<GlyphInfo> glyphs;
        glyphs.reserve(glyphCount);
        for (unsigned int index = 0; index < glyphCount; ++index) {
            const auto& info = glyphInfos[index];
            const auto& position = positions[index];
            glyphs.push_back({
                info.codepoint,
                static_cast<float>(position.x_advance) / kHarfBuzzUnitScale,
                static_cast<float>(position.y_advance) / kHarfBuzzUnitScale,
                static_cast<float>(position.x_offset) / kHarfBuzzUnitScale,
                static_cast<float>(position.y_offset) / kHarfBuzzUnitScale,
                info.cluster,
            });
        }

        hb_buffer_destroy(buffer);
        hb_font_destroy(font);
        return glyphs;
    }

private:
    const FontDatabase& m_fonts;
};

}

std::unique_ptr<TextShaper> CreateHarfBuzzTextShaper(const FontDatabase& fonts)
{
    return std::make_unique<HarfBuzzTextShaper>(fonts);
}

}