#include "bridge.h"

int UsePlatformDeclarations()
{
    HWND window = nullptr;
    return WindowsOnlyCall() + (window == nullptr ? 0 : 1);
}