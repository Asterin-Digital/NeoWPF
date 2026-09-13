/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>

namespace NeoWPF::Diagnostics {

struct CallStackEntry {
    void* address = nullptr;
    char symbol[256] = {0};
};

int CaptureCallStack(CallStackEntry* entries, int maxEntries, int skipFrames = 0) noexcept;

}