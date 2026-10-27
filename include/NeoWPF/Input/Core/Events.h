/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <chrono>

namespace NeoWPF::Input::Core
{
    enum class PointerDeviceType : std::uint8_t
    {
        Mouse,
        Touch,
        Pen,
    };

    enum class PointerPhase : std::uint8_t
    {
        Enter,
        Move,
        Down,
        Up,
        Cancel,
        Leave,
        Wheel,
    };

    /*
        @summary
        Base arguments for input events.
    */
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

    /*
        @summary
        Carries one device-neutral pointer packet from a presentation host.

        @internal
    */
    class PointerEventArgs : public virtual InputEventArgs
    {
    public:
        /*
            @summary
            Creates a pointer packet.

            @internal
        */
        PointerEventArgs(std::uint32_t pointerId, PointerDeviceType deviceType, PointerPhase phase,
                         double x, double y, std::uint32_t buttons = 0,
                         std::uint32_t changedButton = 0, float pressure = 0.0f,
                         double wheelDelta = 0.0);

        std::uint32_t GetPointerId() const noexcept { return m_pointerId; }
        PointerDeviceType GetDeviceType() const noexcept { return m_deviceType; }
        PointerPhase GetPhase() const noexcept { return m_phase; }
        double GetX() const noexcept { return m_x; }
        double GetY() const noexcept { return m_y; }
        std::uint32_t GetButtons() const noexcept { return m_buttons; }
        std::uint32_t GetChangedButton() const noexcept { return m_changedButton; }
        float GetPressure() const noexcept { return m_pressure; }
        double GetWheelDelta() const noexcept { return m_wheelDelta; }

    private:
        std::uint32_t m_pointerId;
        PointerDeviceType m_deviceType;
        PointerPhase m_phase;
        double m_x;
        double m_y;
        std::uint32_t m_buttons;
        std::uint32_t m_changedButton;
        float m_pressure;
        double m_wheelDelta;
    };

    /*
        @summary
        Carries keyboard state and character information.
    */
    class KeyboardEventArgs : public virtual InputEventArgs
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
