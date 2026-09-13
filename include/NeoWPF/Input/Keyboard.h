/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Input/InputManager.h>

namespace NeoWPF::Input
{
    enum class KeyState
    {
        Up = 0,
        Down = 1
    };

    enum class Key : int
    {
        None = 0,
        Back = 0x08,
        Tab = 0x09,
        Enter = 0x0D,
        Shift = 0x10,
        Control = 0x11,
        Alt = 0x12,
        Pause = 0x13,
        CapsLock = 0x14,
        Escape = 0x1B,
        Space = 0x20,
        PageUp = 0x21,
        PageDown = 0x22,
        End = 0x23,
        Home = 0x24,
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,
        Insert = 0x2D,
        Delete = 0x2E,
        D0 = 0x30,
        D1 = 0x31,
        D2 = 0x32,
        D3 = 0x33,
        D4 = 0x34,
        D5 = 0x35,
        D6 = 0x36,
        D7 = 0x37,
        D8 = 0x38,
        D9 = 0x39,
        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A,
    };

    class Keyboard
    {
    public:
        static KeyState GetKeyState(int keyCode)
        {
            return InputManager::GetInstance().IsKeyDown(keyCode) ? KeyState::Down : KeyState::Up;
        }

        static KeyState GetKeyState(Key key)
        {
            return GetKeyState(static_cast<int>(key));
        }

        static KeyState GetKeyState(char keyChar)
        {
            return GetKeyState(FromChar(keyChar));
        }

        static KeyState GetKeyState(wchar_t keyChar)
        {
            return GetKeyState(FromChar(keyChar));
        }

        static bool IsKeyDown(int keyCode)
        {
            return InputManager::GetInstance().IsKeyDown(keyCode);
        }

        static bool IsKeyDown(Key key)
        {
            return IsKeyDown(static_cast<int>(key));
        }

        static bool IsKeyDown(char keyChar)
        {
            return IsKeyDown(FromChar(keyChar));
        }

        static bool IsKeyDown(wchar_t keyChar)
        {
            return IsKeyDown(FromChar(keyChar));
        }

        static bool IsKeyUp(int keyCode)
        {
            return InputManager::GetInstance().IsKeyUp(keyCode);
        }

        static bool IsKeyUp(Key key)
        {
            return IsKeyUp(static_cast<int>(key));
        }

        static bool IsKeyUp(char keyChar)
        {
            return IsKeyUp(FromChar(keyChar));
        }

        static bool IsKeyUp(wchar_t keyChar)
        {
            return IsKeyUp(FromChar(keyChar));
        }

    private:
        static Key FromChar(char keyChar)
        {
            return FromChar(static_cast<wchar_t>(keyChar));
        }

        static Key FromChar(wchar_t keyChar)
        {
            if (keyChar >= L'a' && keyChar <= L'z')
            {
                return static_cast<Key>(keyChar - 32);
            }
            if (keyChar >= L'A' && keyChar <= L'Z')
            {
                return static_cast<Key>(keyChar);
            }
            if (keyChar >= L'0' && keyChar <= L'9')
            {
                return static_cast<Key>(keyChar);
            }
            return Key::None;
        }
    };
}
