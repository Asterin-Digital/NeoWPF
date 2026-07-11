#include <NeoWPF/Input/Core/Events.h>
#include <chrono>

namespace NeoWPF::Input::Core
{
    InputEventArgs::InputEventArgs()
    {
        auto now = std::chrono::high_resolution_clock::now();
        m_timestamp = now.time_since_epoch().count();
    }

    MouseEventArgs::MouseEventArgs(int x, int y, std::uint32_t buttonState, int wheelDelta)
        : InputEventArgs(), m_x(x), m_y(y), m_buttonState(buttonState), m_wheelDelta(wheelDelta)
    {
    }

    KeyboardEventArgs::KeyboardEventArgs(int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
        : InputEventArgs(), m_keyCode(keyCode), m_isKeyDown(isKeyDown), m_charCode(charCode), m_repeatCount(repeatCount)
    {
    }
}
