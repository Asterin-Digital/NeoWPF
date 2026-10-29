/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Text/TextTypes.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace NeoWPF::Rendering::Text {

/*
    @summary
    Stores registered fonts and resolves font requests.
*/
class FontDatabase {
public:
    FontDatabase();
    ~FontDatabase();

    FontDatabase(const FontDatabase&) = delete;
    FontDatabase& operator=(const FontDatabase&) = delete;

    /*
        @summary
        Registers a font from a file.

        @param path
        Font file path.

        @returns
        True when registration succeeds.
    */
    bool RegisterFile(const std::filesystem::path& path);
    /*
        @summary
        Registers font data from memory.

        @param data
        Shared font byte buffer.

        @param familyUtf8
        Font family name encoded as UTF-8.

        @returns
        True when registration succeeds.
    */
    bool RegisterMemory(std::shared_ptr<const std::vector<std::byte>> data,
                        std::string familyUtf8);
    /*
        @summary
        Resolves a font request to a registered font record.

        @param request
        Font characteristics to resolve.
    */
    std::shared_ptr<const FontRecord> Resolve(FontRequest request) const;
    /*
        @summary
        Returns the registered font records.
    */
    std::vector<std::shared_ptr<const FontRecord>> Records() const;

    /*
        @summary
        Returns the generation incremented when a font is registered.

        @internal
    */
    std::uint64_t Generation() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/*
    @summary
    Returns the shared font database.

    @internal
*/
FontDatabase& GetFontDatabase();

}
