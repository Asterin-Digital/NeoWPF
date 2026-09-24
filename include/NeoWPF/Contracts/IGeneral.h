/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace NeoWPF::Contracts {

struct FileDialogFilter {
    std::wstring Name;
    std::vector<std::wstring> Extensions;
};

struct FileDialogResult {
    bool Succeeded = false;
    std::wstring FileName;
    std::vector<std::wstring> FileNames;
};

struct ClipboardTextResult {
    bool Succeeded = false;
    std::wstring Text;
};

class IClipboard {
public:
    using TextCallback = std::function<void(ClipboardTextResult)>;

    virtual ~IClipboard() = default;

    virtual bool SetText(const std::wstring& text) = 0;
    virtual void TryGetText(TextCallback callback) = 0;
    virtual bool Clear() = 0;
};

}