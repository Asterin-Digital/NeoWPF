/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Text/TextTypes.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace NeoWPF::Rendering::Text {

class FontDatabase {
public:
    FontDatabase();
    ~FontDatabase();

    FontDatabase(const FontDatabase&) = delete;
    FontDatabase& operator=(const FontDatabase&) = delete;

    bool RegisterFile(const std::filesystem::path& path);
    bool RegisterMemory(std::shared_ptr<const std::vector<std::byte>> data,
                        std::string familyUtf8);
    std::shared_ptr<const FontRecord> Resolve(FontRequest request) const;
    std::vector<std::shared_ptr<const FontRecord>> Records() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

FontDatabase& GetFontDatabase();

}
