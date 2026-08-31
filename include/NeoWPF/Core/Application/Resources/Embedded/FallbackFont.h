#pragma once

#include <cstddef>

namespace NeoWPF::Core::Application::Resources::Embedded {

struct FontRecord {
    const char* name = nullptr;
    const unsigned char* data = nullptr;
    std::size_t size = 0;
};

std::size_t GetEmbeddedFallbackFontCount() noexcept;
const char* GetEmbeddedFallbackFontName(std::size_t index) noexcept;
bool TryGetEmbeddedFallbackFont(std::size_t index, FontRecord& outRecord) noexcept;

}
