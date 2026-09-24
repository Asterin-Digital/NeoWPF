/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::Input
{
    enum class MouseButtonState
    {
        Released = 0,
        Pressed = 1
    };

    class Mouse
    {
    public:
        static int GetX() { return InputManager::GetInstance().GetMouseX(); }
        static int GetY() { return InputManager::GetInstance().GetMouseY(); }

        static std::uint32_t GetButtonState() { return InputManager::GetInstance().GetMouseButtonState(); }

        static MouseButtonState LeftButton()
        {
            return (GetButtonState() & 0x01) != 0 ? MouseButtonState::Pressed : MouseButtonState::Released;
        }

        static MouseButtonState RightButton()
        {
            return (GetButtonState() & 0x02) != 0 ? MouseButtonState::Pressed : MouseButtonState::Released;
        }

        static MouseButtonState MiddleButton()
        {
            return (GetButtonState() & 0x04) != 0 ? MouseButtonState::Pressed : MouseButtonState::Released;
        }

        static bool Capture(NeoWPF::UI::UIElement* element)
        {
            InputManager::GetInstance().SetMouseCapture(element);
            return true;
        }

        static NeoWPF::UI::UIElement* Captured()
        {
            return InputManager::GetInstance().GetMouseCapture();
        }

        static NeoWPF::UI::UIElement* DirectlyOver()
        {
            return InputManager::GetInstance().GetMouseOverElement();
        }

        static NeoWPF::UI::Core::Point GetPosition(const NeoWPF::UI::UIElement* relativeTo = nullptr)
        {
            const int x = GetX();
            const int y = GetY();
            if (!relativeTo)
            {
                return NeoWPF::UI::Core::Point(static_cast<float>(x), static_cast<float>(y));
            }

            if (auto fw = dynamic_cast<const NeoWPF::UI::FrameworkElement*>(relativeTo))
            {
                double absX = 0.0;
                double absY = 0.0;
                fw->GetAbsolutePosition(absX, absY);
                return NeoWPF::UI::Core::Point(static_cast<float>(x - absX), static_cast<float>(y - absY));
            }

            return NeoWPF::UI::Core::Point(static_cast<float>(x), static_cast<float>(y));
        }
    };
}
