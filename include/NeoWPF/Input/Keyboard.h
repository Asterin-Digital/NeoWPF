/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Input/InputManager.h>

namespace NeoWPF::Input
{
    /*
        @summary
        Describes whether a key is pressed or released.
    */
    enum class KeyState
    {
        Up = 0,
        Down = 1
    };

    /*
        @summary
        Identifies keyboard keys using NeoWPF's platform-independent key codes.

        @details
        Values are explicitly grouped into control, modifier, navigation, editing, digit, letter, function, numpad, punctuation, and media ranges.
    */
    enum class Key : int
    {
        None = 0,
        Back = 1,
        Tab = 2,
        Enter = 3,
        Pause = 4,
        CapsLock = 5,
        Escape = 6,
        Shift = 10,
        Control = 11,
        Alt = 12,
        PageUp = 20,
        PageDown = 21,
        End = 22,
        Home = 23,
        Left = 24,
        Up = 25,
        Right = 26,
        Down = 27,
        Insert = 30,
        Delete = 31,
        D0 = 40,
        D1 = 41,
        D2 = 42,
        D3 = 43,
        D4 = 44,
        D5 = 45,
        D6 = 46,
        D7 = 47,
        D8 = 48,
        D9 = 49,
        A = 60,
        B = 61,
        C = 62,
        D = 63,
        E = 64,
        F = 65,
        G = 66,
        H = 67,
        I = 68,
        J = 69,
        K = 70,
        L = 71,
        M = 72,
        N = 73,
        O = 74,
        P = 75,
        Q = 76,
        R = 77,
        S = 78,
        T = 79,
        U = 80,
        V = 81,
        W = 82,
        X = 83,
        Y = 84,
        Z = 85,
        F1 = 90,
        F2 = 91,
        F3 = 92,
        F4 = 93,
        F5 = 94,
        F6 = 95,
        F7 = 96,
        F8 = 97,
        F9 = 98,
        F10 = 99,
        F11 = 100,
        F12 = 101,
        F13 = 102,
        F14 = 103,
        F15 = 104,
        F16 = 105,
        F17 = 106,
        F18 = 107,
        F19 = 108,
        F20 = 109,
        F21 = 110,
        F22 = 111,
        F23 = 112,
        F24 = 113,
        NumPad0 = 120,
        NumPad1 = 121,
        NumPad2 = 122,
        NumPad3 = 123,
        NumPad4 = 124,
        NumPad5 = 125,
        NumPad6 = 126,
        NumPad7 = 127,
        NumPad8 = 128,
        NumPad9 = 129,
        Multiply = 130,
        Add = 131,
        Separator = 132,
        Subtract = 133,
        Decimal = 134,
        Divide = 135,
        Space = 150,
        Oem1 = 151,
        OemPlus = 152,
        OemComma = 153,
        OemMinus = 154,
        OemPeriod = 155,
        Oem2 = 156,
        Oem3 = 157,
        Oem4 = 158,
        Oem5 = 159,
        Oem6 = 160,
        Oem7 = 161,
        Oem8 = 162,
        Oem102 = 163,
        VolumeMute = 180,
        VolumeDown = 181,
        VolumeUp = 182,
        MediaNextTrack = 183,
        MediaPreviousTrack = 184,
        MediaStop = 185,
        MediaPlayPause = 186,
    };

    /*
        @summary
        Provides access to the current keyboard state.
    */
    class Keyboard
    {
    public:
        /*
            @summary
            Returns the state of a key code.

            @param keyCode
            NeoWPF key code to check.

            @returns
            `Down` when pressed; otherwise `Up`.
        */
        static KeyState GetKeyState(int keyCode)
        {
            return InputManager::GetInstance().IsKeyDown(keyCode) ? KeyState::Down : KeyState::Up;
        }

        /*
            @summary
            Returns the state of a key.

            @param key
            Key to check.

            @returns
            `Down` when pressed; otherwise `Up`.
        */
        static KeyState GetKeyState(Key key)
        {
            return GetKeyState(static_cast<int>(key));
        }

        /*
            @summary
            Returns the state of the key associated with a character.

            @param keyChar
            Character identifying the key.

            @returns
            `Down` when pressed; otherwise `Up`.
        */
        static KeyState GetKeyState(char keyChar)
        {
            return GetKeyState(FromChar(keyChar));
        }

        /*
            @summary
            Returns the state of the key associated with a wide character.

            @param keyChar
            Wide character identifying the key.

            @returns
            `Down` when pressed; otherwise `Up`.
        */
        static KeyState GetKeyState(wchar_t keyChar)
        {
            return GetKeyState(FromChar(keyChar));
        }

        /*
            @summary
            Reports whether a key code is pressed.

            @param keyCode
            NeoWPF key code to check.

            @returns
            `true` when the key is pressed; otherwise `false`.
        */
        static bool IsKeyDown(int keyCode)
        {
            return InputManager::GetInstance().IsKeyDown(keyCode);
        }

        /*
            @summary
            Reports whether a key is pressed.

            @param key
            Key to check.

            @returns
            `true` when the key is pressed; otherwise `false`.
        */
        static bool IsKeyDown(Key key)
        {
            return IsKeyDown(static_cast<int>(key));
        }

        /*
            @summary
            Reports whether the key associated with a character is pressed.

            @param keyChar
            Character identifying the key.

            @returns
            `true` when the key is pressed; otherwise `false`.
        */
        static bool IsKeyDown(char keyChar)
        {
            return IsKeyDown(FromChar(keyChar));
        }

        /*
            @summary
            Reports whether the key associated with a wide character is pressed.

            @param keyChar
            Wide character identifying the key.

            @returns
            `true` when the key is pressed; otherwise `false`.
        */
        static bool IsKeyDown(wchar_t keyChar)
        {
            return IsKeyDown(FromChar(keyChar));
        }

        /*
            @summary
            Reports whether a key code is released.

            @param keyCode
            NeoWPF key code to check.

            @returns
            `true` when the key is released; otherwise `false`.
        */
        static bool IsKeyUp(int keyCode)
        {
            return InputManager::GetInstance().IsKeyUp(keyCode);
        }

        /*
            @summary
            Reports whether a key is released.

            @param key
            Key to check.

            @returns
            `true` when the key is released; otherwise `false`.
        */
        static bool IsKeyUp(Key key)
        {
            return IsKeyUp(static_cast<int>(key));
        }

        /*
            @summary
            Reports whether the key associated with a character is released.

            @param keyChar
            Character identifying the key.

            @returns
            `true` when the key is released; otherwise `false`.
        */
        static bool IsKeyUp(char keyChar)
        {
            return IsKeyUp(FromChar(keyChar));
        }

        /*
            @summary
            Reports whether the key associated with a wide character is released.

            @param keyChar
            Wide character identifying the key.

            @returns
            `true` when the key is released; otherwise `false`.
        */
        static bool IsKeyUp(wchar_t keyChar)
        {
            return IsKeyUp(FromChar(keyChar));
        }

    private:
        /*
            @summary
            Converts a narrow character to its key identifier.

            @param keyChar
            Character to convert.

            @returns
            The corresponding key or `Key::None` when it is not mapped.

            @internal
        */
        static Key FromChar(char keyChar)
        {
            return FromChar(static_cast<wchar_t>(keyChar));
        }

        /*
            @summary
            Converts a wide character to its key identifier.

            @param keyChar
            Character to convert.

            @returns
            The corresponding key or `Key::None` when it is not mapped.

            @internal
        */
        static Key FromChar(wchar_t keyChar)
        {
            if (keyChar == L' ')
            {
                return Key::Space;
            }
            switch (keyChar)
            {
            case L';': case L':': return Key::Oem1;
            case L'=': case L'+': return Key::OemPlus;
            case L',': case L'<': return Key::OemComma;
            case L'-': case L'_': return Key::OemMinus;
            case L'.': case L'>': return Key::OemPeriod;
            case L'/': case L'?': return Key::Oem2;
            case L'`': case L'~': return Key::Oem3;
            case L'[': case L'{': return Key::Oem4;
            case L'\\': case L'|': return Key::Oem5;
            case L']': case L'}': return Key::Oem6;
            case L'\'': case L'"': return Key::Oem7;
            default: break;
            }
            if (keyChar >= L'a' && keyChar <= L'z')
            {
                return static_cast<Key>(static_cast<int>(Key::A) + keyChar - L'a');
            }
            if (keyChar >= L'A' && keyChar <= L'Z')
            {
                return static_cast<Key>(static_cast<int>(Key::A) + keyChar - L'A');
            }
            if (keyChar >= L'0' && keyChar <= L'9')
            {
                return static_cast<Key>(static_cast<int>(Key::D0) + keyChar - L'0');
            }
            return Key::None;
        }
    };
}
