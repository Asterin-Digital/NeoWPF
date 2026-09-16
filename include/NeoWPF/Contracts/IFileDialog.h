/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

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

}
