#include <NeoWPF/Platform/Linux/X11/XWindowChrome.h>

#include <NeoWPF/Platform/Linux/LClientSideTitleBar.h>
#include <NeoWPF/Platform/Linux/X11/XWindow.h>
#include <NeoWPF/Platform/Linux/X11/XWindowing.h>

#include <X11/Xatom.h>

namespace NeoWPF::Platform::Linux {

XWindowChrome::XWindowChrome(XWindowing& windowing) noexcept
    : m_windowing(windowing)
{
}

NeoWPF::Contracts::ChromeMode XWindowChrome::DefaultChromeMode() const noexcept
{
    return NeoWPF::Contracts::ChromeMode::Native;
}

bool XWindowChrome::SupportsMode(NeoWPF::Contracts::ChromeMode mode) const noexcept
{
    return mode == NeoWPF::Contracts::ChromeMode::Native ||
        mode == NeoWPF::Contracts::ChromeMode::Custom ||
        mode == NeoWPF::Contracts::ChromeMode::None;
}

bool XWindowChrome::SetChromeMode(void* nativeWindowHandle,
                                  NeoWPF::Contracts::ChromeMode mode) noexcept
{
    if (!nativeWindowHandle || !SupportsMode(mode)) {
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    m_modes[nativeWindowHandle] = mode;
    EnsureClientTitleBar(nativeWindowHandle, mode);
    return true;
}

NeoWPF::Contracts::ChromeMode XWindowChrome::GetChromeMode(
    void* nativeWindowHandle) const noexcept
{
    if (!nativeWindowHandle) {
        return DefaultChromeMode();
    }
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    const auto it = m_modes.find(nativeWindowHandle);
    return it == m_modes.end() ? DefaultChromeMode() : it->second;
}

void XWindowChrome::SetTitleBarHitTest(void*,
                                       NeoWPF::Contracts::ChromeHitTestCallback,
                                       void*) noexcept
{
}

void XWindowChrome::BeginInteractiveMove(void* nativeWindowHandle) noexcept
{
    XWindow* window = m_windowing.FindWindow(nativeWindowHandle);
    if (!window || !window->DisplayHandle()) {
        return;
    }
    Display* display = window->DisplayHandle();
    Atom moveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    int rootX = 0;
    int rootY = 0;
    ::Window root = 0;
    ::Window child = 0;
    unsigned int mask = 0;
    XQueryPointer(display, m_windowing.RootHandle(),
        &root, &child, &rootX, &rootY, &rootX, &rootY, &mask);
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = window->XWindowHandle();
    event.xclient.message_type = moveResize;
    event.xclient.format = 32;
    event.xclient.data.l[0] = rootX;
    event.xclient.data.l[1] = rootY;
    event.xclient.data.l[2] = 8;
    event.xclient.data.l[3] = 1;
    event.xclient.data.l[4] = 1;
    XSendEvent(display, m_windowing.RootHandle(), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(display);
}

void XWindowChrome::BeginInteractiveResize(void* nativeWindowHandle,
                                           NeoWPF::Contracts::ResizeEdge edge) noexcept
{
    XWindow* window = m_windowing.FindWindow(nativeWindowHandle);
    if (!window || !window->DisplayHandle()) {
        return;
    }
    Display* display = window->DisplayHandle();
    Atom moveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    int rootX = 0;
    int rootY = 0;
    ::Window root = 0;
    ::Window child = 0;
    unsigned int mask = 0;
    XQueryPointer(display, m_windowing.RootHandle(),
        &root, &child, &rootX, &rootY, &rootX, &rootY, &mask);
    long direction = 10;
    switch (edge) {
    case NeoWPF::Contracts::ResizeEdge::Left: direction = 6; break;
    case NeoWPF::Contracts::ResizeEdge::Right: direction = 4; break;
    case NeoWPF::Contracts::ResizeEdge::Top: direction = 2; break;
    case NeoWPF::Contracts::ResizeEdge::Bottom: direction = 8; break;
    case NeoWPF::Contracts::ResizeEdge::TopLeft: direction = 0; break;
    case NeoWPF::Contracts::ResizeEdge::TopRight: direction = 1; break;
    case NeoWPF::Contracts::ResizeEdge::BottomLeft: direction = 5; break;
    case NeoWPF::Contracts::ResizeEdge::BottomRight: direction = 7; break;
    }
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = window->XWindowHandle();
    event.xclient.message_type = moveResize;
    event.xclient.format = 32;
    event.xclient.data.l[0] = rootX;
    event.xclient.data.l[1] = rootY;
    event.xclient.data.l[2] = direction;
    event.xclient.data.l[3] = 1;
    event.xclient.data.l[4] = 1;
    XSendEvent(display, m_windowing.RootHandle(), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(display);
}

void XWindowChrome::SetMaximized(void* nativeWindowHandle, bool maximized) noexcept
{
    if (XWindow* window = m_windowing.FindWindow(nativeWindowHandle)) {
        window->Maximize(maximized);
    }
}

bool XWindowChrome::IsZoomed(void* nativeWindowHandle) const noexcept
{
    XWindow* window = m_windowing.FindWindow(nativeWindowHandle);
    return window && window->IsMaximized();
}

bool XWindowChrome::IsMaximized(void* nativeWindowHandle) const noexcept
{
    XWindow* window = m_windowing.FindWindow(nativeWindowHandle);
    return window && window->IsMaximized();
}

void XWindowChrome::SetMinimized(void* nativeWindowHandle) noexcept
{
    if (XWindow* window = m_windowing.FindWindow(nativeWindowHandle)) {
        window->Minimize();
    }
}

bool XWindowChrome::SetNativeCaptionColor(void*, std::uint32_t) noexcept
{
    return false;
}

}
