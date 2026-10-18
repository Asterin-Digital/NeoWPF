/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoPM::UI::Theme {

inline constexpr const wchar_t* FontFamily = L"Noto Sans";

inline constexpr std::uint32_t WindowBackground = 0xFF10181Cu;
inline constexpr std::uint32_t SidebarBackground = 0xFF0B1114u;
inline constexpr std::uint32_t SidebarBorder = 0xFF22343Au;
inline constexpr std::uint32_t Surface = 0xFF162126u;
inline constexpr std::uint32_t TextPrimary = 0xFFE8F2F3u;
inline constexpr std::uint32_t TextOnSidebar = 0xFFE8F2F3u;
inline constexpr std::uint32_t TextMuted = 0xFF90A6ACu;
inline constexpr std::uint32_t Accent = 0xFF2BB8A6u;
inline constexpr std::uint32_t AccentPressed = 0xFF218F83u;
inline constexpr std::uint32_t ButtonBackground = 0xFF162126u;
inline constexpr std::uint32_t ButtonBorder = 0xFF294149u;
inline constexpr std::uint32_t ButtonText = 0xFFE8F2F3u;
inline constexpr std::uint32_t ButtonHoverBackground = 0xFF20343Au;
inline constexpr std::uint32_t ButtonHoverBorder = 0xFF3D6974u;
inline constexpr std::uint32_t ButtonPressedBackground = 0xFF102024u;
inline constexpr std::uint32_t ButtonPressedBorder = 0xFF2BB8A6u;
inline constexpr std::uint32_t ButtonDisabledBackground = 0xFF202A2Eu;
inline constexpr std::uint32_t ButtonDisabledBorder = 0xFF354247u;
inline constexpr std::uint32_t ButtonSelectedBackground = 0xFF214B53u;
inline constexpr std::uint32_t ButtonSelectedBorder = 0xFF2BB8A6u;
inline constexpr std::uint32_t ButtonSelectedText = 0xFFFFFFFFu;

void Register();

}
