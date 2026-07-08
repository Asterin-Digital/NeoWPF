#pragma once

#include <cstddef>

namespace NeoWPF::Diagnostics {

struct CallStackEntry {
    void* address = nullptr;
    char symbol[256] = {0};
};

int CaptureCallStack(CallStackEntry* entries, int maxEntries, int skipFrames = 0) noexcept;

}