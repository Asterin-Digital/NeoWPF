#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <Windows.h>
#include <DbgHelp.h>
#include <cstring>

#pragma comment(lib, "Dbghelp.lib")

namespace NeoWPF::Diagnostics {

int CaptureCallStack(CallStackEntry* entries, int maxEntries, int skipFrames) noexcept
{
    if (!entries || maxEntries <= 0) {
        return 0;
    }

    void* stack[64] = {};
    USHORT frames = CaptureStackBackTrace(static_cast<ULONG>(skipFrames + 1), static_cast<ULONG>(maxEntries), stack, nullptr);
    if (frames == 0) {
        return 0;
    }

    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, TRUE)) {
        for (USHORT i = 0; i < frames; ++i) {
            entries[i].address = stack[i];
            entries[i].symbol[0] = '\0';
        }
        return static_cast<int>(frames);
    }

    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(malloc(sizeof(SYMBOL_INFO) + 256));
    if (!symbol) {
        for (USHORT i = 0; i < frames; ++i) {
            entries[i].address = stack[i];
            entries[i].symbol[0] = '\0';
        }
        SymCleanup(process);
        return static_cast<int>(frames);
    }

    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames && i < static_cast<USHORT>(maxEntries); ++i) {
        entries[i].address = stack[i];
        DWORD64 displacement = 0;
        if (SymFromAddr(process, reinterpret_cast<DWORD64>(stack[i]), &displacement, symbol)) {
            strncpy_s(entries[i].symbol, symbol->Name, _TRUNCATE);
        } else {
            entries[i].symbol[0] = '\0';
        }
    }

    free(symbol);
    SymCleanup(process);
    return static_cast<int>(frames);
}

}