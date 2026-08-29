#include <NeoWPF/Diagnostics/CallStackTracer.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>

namespace NeoWPF::Platform::Linux {

int CaptureCallStackLinux(NeoWPF::Diagnostics::CallStackEntry* entries,
                          int maxEntries, int skipFrames) noexcept
{
    if (!entries || maxEntries <= 0) {
        return 0;
    }

    void* stack[64] = {};
    const int requestedFrames = (std::min)(maxEntries + skipFrames + 1, 64);
    const int frameCount = backtrace(stack, requestedFrames);
    if (frameCount <= 0) {
        return 0;
    }

    char** symbols = backtrace_symbols(stack, frameCount);
    const int firstFrame = (std::min)(skipFrames + 1, frameCount);
    const int capturedFrames = (std::min)(frameCount - firstFrame, maxEntries);
    for (int index = 0; index < capturedFrames; ++index) {
        const int frame = firstFrame + index;
        entries[index].address = stack[frame];
        if (symbols && symbols[frame]) {
            std::strncpy(entries[index].symbol, symbols[frame], sizeof(entries[index].symbol) - 1);
            entries[index].symbol[sizeof(entries[index].symbol) - 1] = '\0';
        } else {
            entries[index].symbol[0] = '\0';
        }
    }

    std::free(symbols);
    return capturedFrames;
}

}