/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>

namespace NeoWPF::Core::ApplicationResources::Embedded {

struct FontRecord {
    const char* name = nullptr;
    const unsigned char* data = nullptr;
    std::size_t size = 0;
};

std::size_t GetEmbeddedFallbackFontCount() noexcept;
const char* GetEmbeddedFallbackFontName(std::size_t index) noexcept;
bool TryGetEmbeddedFallbackFont(std::size_t index, FontRecord& outRecord) noexcept;

}
