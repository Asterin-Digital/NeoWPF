#include <NeoWPF/Diagnostics/CallStackTracer.h>

#include <Windows.h>
#include <DbgHelp.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "Dbghelp.lib")

namespace NeoWPF::Platform {

int CaptureCallStackWindows(NeoWPF::Diagnostics::CallStackEntry* entries,
                            int maxEntries, int skipFrames) noexcept
{
    if (!entries || maxEntries <= 0) {
        return 0;
    }

    void* stack[64] = {};
    const USHORT frames = CaptureStackBackTrace(
        static_cast<ULONG>(skipFrames + 1),
        static_cast<ULONG>(maxEntries),
        stack,
        nullptr);
    if (frames == 0) {
        return 0;
    }

    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, TRUE)) {
        for (USHORT index = 0; index < frames; ++index) {
            entries[index].address = stack[index];
            entries[index].symbol[0] = '\0';
        }
        return static_cast<int>(frames);
    }

    auto* symbol = static_cast<SYMBOL_INFO*>(std::malloc(sizeof(SYMBOL_INFO) + 256));
    if (!symbol) {
        for (USHORT index = 0; index < frames; ++index) {
            entries[index].address = stack[index];
            entries[index].symbol[0] = '\0';
        }
        SymCleanup(process);
        return static_cast<int>(frames);
    }

    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT index = 0; index < frames && index < static_cast<USHORT>(maxEntries); ++index) {
        entries[index].address = stack[index];
        DWORD64 displacement = 0;
        if (SymFromAddr(process, reinterpret_cast<DWORD64>(stack[index]), &displacement, symbol)) {
            const size_t symbolLength = (std::min)(
                std::strlen(symbol->Name),
                sizeof(entries[index].symbol) - 1);
            std::memcpy(entries[index].symbol, symbol->Name, symbolLength);
            entries[index].symbol[symbolLength] = '\0';
        } else {
            entries[index].symbol[0] = '\0';
        }
    }

    std::free(symbol);
    SymCleanup(process);
    return static_cast<int>(frames);
}

}