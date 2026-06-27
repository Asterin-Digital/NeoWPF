#pragma once

#include <cstdint>
#include <chrono>

namespace NeoWPF::Input::Core
{
    class InputEventArgs
    {
    public:
        InputEventArgs();
        virtual ~InputEventArgs() = default;

        bool IsHandled() const { return m_handled; }
        void SetHandled(bool handled) { m_handled = handled; }

        std::uint64_t GetTimestamp() const { return m_timestamp; }

    protected:
        bool m_handled = false;
        std::uint64_t m_timestamp;
    };

    class MouseEventArgs : public InputEventArgs
    {
    public:
        MouseEventArgs(int x, int y, std::uint32_t buttonState, int wheelDelta = 0);

        int GetX() const { return m_x; }
        int GetY() const { return m_y; }
        std::uint32_t GetButtonState() const { return m_buttonState; }
        int GetWheelDelta() const { return m_wheelDelta; }

        bool IsLeftButtonDown() const { return (m_buttonState & 0x01) != 0; }
        bool IsRightButtonDown() const { return (m_buttonState & 0x02) != 0; }
        bool IsMiddleButtonDown() const { return (m_buttonState & 0x04) != 0; }

    private:
        int m_x;
        int m_y;
        std::uint32_t m_buttonState;
        int m_wheelDelta;
    };

    class KeyboardEventArgs : public InputEventArgs
    {
    public:
        KeyboardEventArgs(int keyCode, bool isKeyDown, wchar_t charCode = 0, int repeatCount = 1);

        int GetKeyCode() const { return m_keyCode; }
        bool IsKeyDown() const { return m_isKeyDown; }
        wchar_t GetCharCode() const { return m_charCode; }
        int GetRepeatCount() const { return m_repeatCount; }

    private:
        int m_keyCode;
        bool m_isKeyDown;
        wchar_t m_charCode;
        int m_repeatCount;
    };
}
