#include <NeoWPF/Platform/Linux/X11/XClipboard.h>

#include <NeoWPF/Platform/PlatformUtils.h>

#include <X11/Xatom.h>

#include <iterator>
#include <utility>

namespace NeoWPF::Platform::Linux {

XClipboard::XClipboard(XWindowing& windowing) noexcept
    : m_windowing(windowing)
{
    Display* display = m_windowing.DisplayHandle();
    if (!display) {
        return;
    }
    m_clipboard = XInternAtom(display, "CLIPBOARD", False);
    m_utf8 = XInternAtom(display, "UTF8_STRING", False);
    m_targets = XInternAtom(display, "TARGETS", False);
    m_owner = XCreateSimpleWindow(
        display,
        m_windowing.RootHandle(),
        -1,
        -1,
        1,
        1,
        0,
        0,
        0);
    XSelectInput(display, m_owner, PropertyChangeMask);
    m_windowing.AttachClipboard(this);
}

XClipboard::~XClipboard()
{
    m_windowing.DetachClipboard(this);
    if (m_owner && m_windowing.DisplayHandle()) {
        XDestroyWindow(m_windowing.DisplayHandle(), m_owner);
    }
}

NeoWPF::Result<void> XClipboard::SetText(const std::wstring& text)
{
    Display* display = m_windowing.DisplayHandle();
    if (!display || !m_owner) {
        return NeoWPF::Fail<std::string>{"X11 clipboard owner could not be created"};
    }
    m_ownedText = text;
    XSetSelectionOwner(display, m_clipboard, m_owner, CurrentTime);
    if (XGetSelectionOwner(display, m_clipboard) != m_owner) {
        return NeoWPF::Fail<std::string>{"X11 clipboard ownership was rejected"};
    }
    XFlush(display);
    return NeoWPF::Success<void>{};
}

void XClipboard::TryGetText(TextCallback callback)
{
    m_pendingCallback = std::move(callback);
    Display* display = m_windowing.DisplayHandle();
    if (!display || !m_owner) {
        if (m_pendingCallback) {
            m_pendingCallback(NeoWPF::Fail<std::string>{"X11 clipboard is unavailable"});
        }
        m_pendingCallback = {};
        return;
    }
    const Atom property = XInternAtom(display, "NEOWPF_CLIPBOARD", False);
    XConvertSelection(display, m_clipboard, m_utf8, property, m_owner, CurrentTime);
    XFlush(display);
}

NeoWPF::Result<void> XClipboard::Clear()
{
    Display* display = m_windowing.DisplayHandle();
    if (!display || !m_owner) {
        return NeoWPF::Fail<std::string>{"X11 clipboard owner could not be created"};
    }
    XSetSelectionOwner(display, m_clipboard, 0, CurrentTime);
    m_ownedText.clear();
    XFlush(display);
    return NeoWPF::Success<void>{};
}

void XClipboard::ProcessSelectionRequest(const XEvent& event) noexcept
{
    Display* display = m_windowing.DisplayHandle();
    if (!display) {
        return;
    }
    const XSelectionRequestEvent& request = event.xselectionrequest;
    XSelectionEvent response{};
    response.type = SelectionNotify;
    response.requestor = request.requestor;
    response.selection = request.selection;
    response.target = request.target;
    response.property = request.property;
    response.time = request.time;

    if (XGetSelectionOwner(display, m_clipboard) == m_owner &&
        (request.target == m_utf8 || request.target == XA_STRING)) {
        const std::string text = NeoWPF::Platform::WideToUtf8(m_ownedText);
        XChangeProperty(display, request.requestor, request.property,
            request.target == m_utf8 ? m_utf8 : XA_STRING,
            8,
            PropModeReplace,
            reinterpret_cast<const unsigned char*>(text.data()),
            static_cast<int>(text.size()));
    } else if (request.target == m_targets) {
        const Atom targets[] = {m_targets, m_utf8, XA_STRING};
        XChangeProperty(display, request.requestor, request.property, XA_ATOM,
            32,
            PropModeReplace,
            reinterpret_cast<const unsigned char*>(targets),
            static_cast<int>(std::size(targets)));
    } else {
        response.property = 0;
    }
    XSendEvent(display, request.requestor, True, NoEventMask,
        reinterpret_cast<XEvent*>(&response));
    XFlush(display);
}

void XClipboard::ProcessSelectionNotify(const XEvent& event) noexcept
{
    Display* display = m_windowing.DisplayHandle();
    if (!display || !m_pendingCallback) {
        return;
    }
    const XSelectionEvent& selection = event.xselection;
    if (selection.property == 0) {
        m_pendingCallback(NeoWPF::Success<std::optional<std::wstring>>{std::nullopt});
        m_pendingCallback = {};
        return;
    }
    Atom actualType = 0;
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long remaining = 0;
    unsigned char* data = nullptr;
    if (XGetWindowProperty(display, selection.requestor, selection.property,
            0,
            1024 * 1024,
            True,
            AnyPropertyType,
            &actualType,
            &actualFormat,
            &itemCount,
            &remaining,
            &data) == 0 && data) {
        if (itemCount > 0) {
            std::string text(reinterpret_cast<const char*>(data), itemCount);
            m_pendingCallback(NeoWPF::Success<std::optional<std::wstring>>{
                NeoWPF::Platform::Utf8ToWide(text)});
        } else {
            m_pendingCallback(NeoWPF::Success<std::optional<std::wstring>>{std::nullopt});
        }
        XFree(data);
    } else {
        m_pendingCallback(NeoWPF::Fail<std::string>{"X11 clipboard read failed"});
    }
    m_pendingCallback = {};
}

}
