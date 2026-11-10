/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <NeoWPF/Etc/Result.h>

namespace NeoWPF::Contracts {

struct FileDialogFilter {
    std::wstring Name;
    std::vector<std::wstring> Extensions;
};

/*
    @summary
    Contains files selected by a completed file dialog.
*/
struct FileDialogSelection {
    std::wstring FileName;
    std::vector<std::wstring> FileNames;
};

/*
    @summary
    Represents a file dialog selection, cancellation, or failure.
*/
using FileDialogResult = NeoWPF::Result<std::optional<FileDialogSelection>>;

/*
    @summary
    Represents clipboard text, no available text, or a read failure.
*/
using ClipboardTextResult = NeoWPF::Result<std::optional<std::wstring>>;

/*
    @summary
    Contains decoded RGBA image pixels and their dimensions.
*/
struct DecodedImage {
    std::vector<std::uint8_t> Pixels;
    int Width = 0;
    int Height = 0;
};

class IClipboard {
public:
    using TextCallback = std::function<void(ClipboardTextResult)>;

    virtual ~IClipboard() = default;

    virtual NeoWPF::Result<void> SetText(const std::wstring& text) = 0;
    virtual void TryGetText(TextCallback callback) = 0;
    virtual NeoWPF::Result<void> Clear() = 0;
};

}