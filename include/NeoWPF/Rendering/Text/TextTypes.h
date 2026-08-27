#pragma once

#include <NeoWPF/UI/Text/FontStyle.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::Rendering::Text {

struct FontRequest {
    std::string familyUtf8;
    int weight = 400;
    NeoWPF::UI::FontStyle style = NeoWPF::UI::FontStyle::Normal;
};

struct FontRecord {
    std::string familyUtf8;
    int weight = 400;
    NeoWPF::UI::FontStyle style = NeoWPF::UI::FontStyle::Normal;
    unsigned int faceIndex = 0;
    std::shared_ptr<const std::vector<std::byte>> data;
};

struct TextMetrics {
    int width = 0;
    int height = 0;
    int lineHeight = 0;
    int baseline = 0;
};

struct GlyphInfo {
    std::uint32_t glyphId = 0;
    float xAdvance = 0.0f;
    float yAdvance = 0.0f;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    std::size_t textOffset = 0;
};

struct TextBitmap {
    int width = 0;
    int height = 0;
    int stride = 0;
    int inkTop = 0;
    int inkHeight = 0;
    std::vector<std::uint8_t> rgba;
};

}
