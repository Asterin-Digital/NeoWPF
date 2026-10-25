#include "bridge.h"

int UsePlatformDeclaration()
{
    // neolint-disable-next-line neowpf-platform-dependency-isolation: Windows API use is intentionally isolated here.
    return WindowsOnlyCall();
}