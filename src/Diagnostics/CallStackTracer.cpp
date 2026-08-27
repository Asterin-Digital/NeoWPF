#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

namespace NeoWPF::Diagnostics {

int CaptureCallStack(CallStackEntry* entries, int maxEntries, int skipFrames) noexcept
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform ? platform->CaptureCallStack(entries, maxEntries, skipFrames) : 0;
}

}