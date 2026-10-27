#include <NeoWPF/Input/Core/Events.h>
#include <chrono>

namespace NeoWPF::Input::Core
{
    InputEventArgs::InputEventArgs()
    {
        auto now = std::chrono::high_resolution_clock::now();
        m_timestamp = now.time_since_epoch().count();
    }

    PointerEventArgs::PointerEventArgs(std::uint32_t pointerId, PointerDeviceType deviceType,
                                       PointerPhase phase,
                                       double x, double y, std::uint32_t buttons,
                                       std::uint32_t changedButton, float pressure,
                                       double wheelDelta)
                : InputEventArgs(), m_pointerId(pointerId), m_deviceType(deviceType),
                    m_phase(phase), m_x(x), m_y(y),
          m_buttons(buttons), m_changedButton(changedButton), m_pressure(pressure),
          m_wheelDelta(wheelDelta)
    {
    }

    KeyboardEventArgs::KeyboardEventArgs(int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
        : InputEventArgs(), m_keyCode(keyCode), m_isKeyDown(isKeyDown), m_charCode(charCode), m_repeatCount(repeatCount)
    {
    }
}
