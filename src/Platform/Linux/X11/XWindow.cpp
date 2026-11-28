#include <NeoWPF/Platform/Linux/X11/XWindow.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Platform/Linux/LKeyboard.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace NeoWPF::Platform::Linux {
namespace {

void SubmitPointerInput(NeoWPF::Input::Core::PointerPhase phase,
                        double x, double y, std::uint32_t buttons,
                        std::uint32_t changedButton = 0, double wheelDelta = 0.0)
{
    auto& input = static_cast<NeoWPF::Contracts::IPointerInput&>(
        NeoWPF::Input::InputManager::GetInstance());
    input.SubmitPointerEvent(NeoWPF::Input::Core::PointerEventArgs(
        0,
        NeoWPF::Input::Core::PointerDeviceType::Mouse,
        phase,
        x,
        y,
        buttons,
        changedButton,
        0.0f,
        wheelDelta));
}

std::uint32_t XButtonToNeoWPFButton(unsigned int button) noexcept
{
    switch (button) {
    case Button1: return 0x01;
    case Button2: return 0x04;
    case Button3: return 0x02;
    default: return 0;
    }
}

}

XWindow::XWindow(XWindowing& windowing, const wchar_t* title, int width, int height,
                 bool transparent)
    : m_windowing(windowing),
      m_display(windowing.DisplayHandle()),
      m_title(title ? title : L""),
      m_width(std::max(width, 1)),
      m_height(std::max(height, 1)),
      m_transparent(transparent)
{
    if (!m_display) {
        return;
    }
    m_window = XCreateSimpleWindow(
        m_display,
        m_windowing.RootHandle(),
        0,
        0,
        static_cast<unsigned int>(m_width),
        static_cast<unsigned int>(m_height),
        0,
        BlackPixel(m_display, DefaultScreen(m_display)),
        WhitePixel(m_display, DefaultScreen(m_display)));
    if (!m_window) {
        return;
    }

    const auto titleUtf8 = NeoWPF::Platform::WideToUtf8(m_title);
    if (titleUtf8.Fail()) {
        NeoWPF::Diagnostics::LogWarn(
            "X11 window title conversion failed: %s", titleUtf8.Error().c_str());
    } else {
        XStoreName(m_display, m_window, titleUtf8.Value().c_str());
    }
    XClassHint classHint{};
    classHint.res_name = const_cast<char*>("NeoWPF");
    classHint.res_class = const_cast<char*>("NeoWPF");
    XSetClassHint(m_display, m_window, &classHint);

    XSizeHints* sizeHints = XAllocSizeHints();
    if (sizeHints) {
        sizeHints->flags = PMinSize | PResizeInc;
        sizeHints->min_width = 1;
        sizeHints->min_height = 1;
        sizeHints->width_inc = 1;
        sizeHints->height_inc = 1;
        XSetWMNormalHints(m_display, m_window, sizeHints);
        XFree(sizeHints);
    }

    const Atom deleteWindow = m_windowing.WmDeleteWindowAtom();
    XSetWMProtocols(m_display, m_window, const_cast<Atom*>(&deleteWindow), 1);
    XSelectInput(m_display, m_window,
        ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
        EnterWindowMask | LeaveWindowMask | FocusChangeMask | PropertyChangeMask);
    XMapWindow(m_display, m_window);
    XFlush(m_display);

    const int screen = DefaultScreen(m_display);
    const int widthMm = DisplayWidthMM(m_display, screen);
    const int widthPixels = DisplayWidth(m_display, screen);
    if (widthMm > 0 && widthPixels > 0) {
        m_scaleFactor = (25.4 * static_cast<double>(widthPixels)) /
            (static_cast<double>(widthMm) * 96.0);
        if (!std::isfinite(m_scaleFactor) || m_scaleFactor <= 0.0) {
            m_scaleFactor = 1.0;
        }
    }
}

XWindow::~XWindow()
{
    Close();
}

bool XWindow::IsValid() const noexcept
{
    return m_display != nullptr && m_window != 0;
}

void XWindow::Close() noexcept
{
    if (m_window && m_display) {
        XDestroyWindow(m_display, m_window);
        XFlush(m_display);
        m_window = 0;
    }
    m_closed = true;
}

void XWindow::Resize(int width, int height) noexcept
{
    m_width = std::max(width, 1);
    m_height = std::max(height, 1);
    if (m_display && m_window) {
        XResizeWindow(m_display, m_window,
            static_cast<unsigned int>(m_width),
            static_cast<unsigned int>(m_height));
        XFlush(m_display);
    }
}

void XWindow::Show() noexcept
{
    if (m_display && m_window) {
        XMapRaised(m_display, m_window);
        XFlush(m_display);
    }
}

void XWindow::Hide() noexcept
{
    if (m_display && m_window) {
        XUnmapWindow(m_display, m_window);
        XFlush(m_display);
    }
}

void XWindow::Minimize() noexcept
{
    if (m_display && m_window) {
        XIconifyWindow(m_display, m_window, DefaultScreen(m_display));
        XFlush(m_display);
    }
}

void XWindow::Maximize(bool maximize) noexcept
{
    if (!m_display || !m_window) {
        return;
    }
    Atom wmState = XInternAtom(m_display, "_NET_WM_STATE", False);
    Atom wmStateMaximizedHorizontal = XInternAtom(
        m_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom wmStateMaximizedVertical = XInternAtom(
        m_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = m_window;
    event.xclient.message_type = wmState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = maximize ? 1 : 0;
    event.xclient.data.l[1] = static_cast<long>(wmStateMaximizedHorizontal);
    event.xclient.data.l[2] = static_cast<long>(wmStateMaximizedVertical);
    XSendEvent(m_display, m_windowing.RootHandle(), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &event);
    m_maximized = maximize;
    XFlush(m_display);
}

bool XWindow::IsClosed() const noexcept
{
    return m_closed;
}

bool XWindow::IsMaximized() const noexcept
{
    return m_maximized;
}

int XWindow::Width() const noexcept
{
    return m_width;
}

int XWindow::Height() const noexcept
{
    return m_height;
}

double XWindow::ScaleFactor() const noexcept
{
    return m_scaleFactor;
}

void* XWindow::NativeHandle() const noexcept
{
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(m_window));
}

::Window XWindow::XWindowHandle() const noexcept
{
    return m_window;
}

Display* XWindow::DisplayHandle() const noexcept
{
    return m_display;
}

void XWindow::HandleEvent(XEvent& event) noexcept
{
    switch (event.type) {
    case ConfigureNotify:
        m_width = std::max(static_cast<int>(event.xconfigure.width), 1);
        m_height = std::max(static_cast<int>(event.xconfigure.height), 1);
        break;
    case EnterNotify:
        break;
    case LeaveNotify:
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Leave,
            static_cast<double>(event.xcrossing.x),
            static_cast<double>(event.xcrossing.y),
            0);
        break;
    case MotionNotify:
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Move,
            static_cast<double>(event.xmotion.x),
            static_cast<double>(event.xmotion.y),
            0);
        break;
    case ButtonPress: {
        const std::uint32_t button = XButtonToNeoWPFButton(event.xbutton.button);
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Down,
            static_cast<double>(event.xbutton.x),
            static_cast<double>(event.xbutton.y),
            button,
            button);
        break;
    }
    case ButtonRelease: {
        const std::uint32_t button = XButtonToNeoWPFButton(event.xbutton.button);
        SubmitPointerInput(NeoWPF::Input::Core::PointerPhase::Up,
            static_cast<double>(event.xbutton.x),
            static_cast<double>(event.xbutton.y),
            0,
            button);
        break;
    }
    case KeyPress: {
        char text[32]{};
        KeySym keysym = XLookupKeysym(&event.xkey, 0);
        XLookupString(&event.xkey, text, sizeof(text), &keysym, nullptr);
        const std::uint32_t virtualKey = TranslateXkbKeysymToVirtualKey(
            static_cast<std::uint32_t>(keysym));
        std::wstring wideText;
        if (text[0] != '\0') {
            auto convertedText = NeoWPF::Platform::Utf8ToWide(text);
            if (convertedText.Fail()) {
                NeoWPF::Diagnostics::LogWarn(
                    "X11 keyboard text conversion failed: %s",
                    convertedText.Error().c_str());
            } else {
                wideText = std::move(convertedText).Value();
            }
        }
        const wchar_t character = wideText.empty() ? L'\0' : wideText[0];
        NeoWPF::Core::SetInputRoot(this);
        NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
            virtualKey == 0 ? -1 : static_cast<int>(virtualKey), true, character, 1);
        m_repeatKey = static_cast<std::uint32_t>(event.xkey.keycode);
        m_repeatCharacter = character;
        m_repeatActive = true;
        m_repeatNext = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
        break;
    }
    case KeyRelease:
        if (m_repeatKey == event.xkey.keycode) {
            m_repeatActive = false;
            m_repeatKey = 0;
            m_repeatCharacter = L'\0';
        }
        break;
    case ClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == m_windowing.WmDeleteWindowAtom()) {
            m_closed = true;
        }
        break;
    default:
        break;
    }
}

void XWindow::ProcessKeyRepeat() noexcept
{
    if (!m_repeatActive || !m_display) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    if (now < m_repeatNext) {
        return;
    }
    KeySym keysym = XkbKeycodeToKeysym(m_display, m_repeatKey, 0, 0);
    const std::uint32_t virtualKey = TranslateXkbKeysymToVirtualKey(
        static_cast<std::uint32_t>(keysym));
    NeoWPF::Core::SetInputRoot(this);
    NeoWPF::Input::InputManager::GetInstance().ProcessKeyboardInput(
        virtualKey == 0 ? -1 : static_cast<int>(virtualKey), true, m_repeatCharacter, 1);
    m_repeatNext = now + std::chrono::milliseconds(40);
}

}
