/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

struct Color {
    std::uint8_t A = 0xFF;
    std::uint8_t R = 0x00;
    std::uint8_t G = 0x00;
    std::uint8_t B = 0x00;

    constexpr Color() noexcept = default;

    constexpr Color(std::uint8_t a, std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        : A(a), R(r), G(g), B(b) {}

    constexpr Color(std::uint32_t argb) noexcept
        : A(static_cast<std::uint8_t>((argb >> 24) & 0xFFu)),
          R(static_cast<std::uint8_t>((argb >> 16) & 0xFFu)),
          G(static_cast<std::uint8_t>((argb >> 8) & 0xFFu)),
          B(static_cast<std::uint8_t>(argb & 0xFFu)) {}

    constexpr std::uint32_t ToArgb() const noexcept {
        return (static_cast<std::uint32_t>(A) << 24) |
               (static_cast<std::uint32_t>(R) << 16) |
               (static_cast<std::uint32_t>(G) << 8) |
               static_cast<std::uint32_t>(B);
    }

    static constexpr Color FromArgb(std::uint32_t argb) noexcept {
        return Color(argb);
    }

    static constexpr Color FromRgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        return Color(0xFF, r, g, b);
    }

    constexpr bool operator==(const Color& other) const noexcept {
        return A == other.A && R == other.R && G == other.G && B == other.B;
    }

    constexpr bool operator!=(const Color& other) const noexcept {
        return !(*this == other);
    }
};

struct Colors {
    static std::shared_ptr<Brush> Black() { return std::make_shared<SolidColorBrush>(0xFF000000u); }
    static std::shared_ptr<Brush> White() { return std::make_shared<SolidColorBrush>(0xFFFFFFFFu); }
    static std::shared_ptr<Brush> Transparent() { return std::make_shared<SolidColorBrush>(0x00000000u); }

    //Grays 
    static std::shared_ptr<Brush> Gray100() { return std::make_shared<SolidColorBrush>(0xFFF7FAFCu); }
    static std::shared_ptr<Brush> Gray200() { return std::make_shared<SolidColorBrush>(0xFFEDF2F7u); }
    static std::shared_ptr<Brush> Gray300() { return std::make_shared<SolidColorBrush>(0xFFE2E8F0u); }
    static std::shared_ptr<Brush> Gray400() { return std::make_shared<SolidColorBrush>(0xFFCBD5E0u); }
    static std::shared_ptr<Brush> Gray500() { return std::make_shared<SolidColorBrush>(0xFFA0AEC0u); }
    static std::shared_ptr<Brush> Gray600() { return std::make_shared<SolidColorBrush>(0xFF718096u); }
    static std::shared_ptr<Brush> Gray700() { return std::make_shared<SolidColorBrush>(0xFF4A5568u); }
    static std::shared_ptr<Brush> Gray800() { return std::make_shared<SolidColorBrush>(0xFF2D3748u); }
    static std::shared_ptr<Brush> Gray900() { return std::make_shared<SolidColorBrush>(0xFF1A202Cu); }

    // Reds
    static std::shared_ptr<Brush> Red() { return std::make_shared<SolidColorBrush>(0xFFFF0000u); }
    static std::shared_ptr<Brush> Red50() { return std::make_shared<SolidColorBrush>(0xFFFFF5F5u); }
    static std::shared_ptr<Brush> Red100() { return std::make_shared<SolidColorBrush>(0xFFFFEBEEu); }
    static std::shared_ptr<Brush> Red200() { return std::make_shared<SolidColorBrush>(0xFFFFCDD2u); }
    static std::shared_ptr<Brush> Red300() { return std::make_shared<SolidColorBrush>(0xFFEF9A9Au); }
    static std::shared_ptr<Brush> Red400() { return std::make_shared<SolidColorBrush>(0xFFEF5350u); }
    static std::shared_ptr<Brush> Red500() { return std::make_shared<SolidColorBrush>(0xFFF44336u); }
    static std::shared_ptr<Brush> Red600() { return std::make_shared<SolidColorBrush>(0xFFE53935u); }
    static std::shared_ptr<Brush> Red700() { return std::make_shared<SolidColorBrush>(0xFFD32F2Fu); }
    static std::shared_ptr<Brush> Red800() { return std::make_shared<SolidColorBrush>(0xFFC62828u); }
    static std::shared_ptr<Brush> Red900() { return std::make_shared<SolidColorBrush>(0xFFB71C1Cu); }
    static std::shared_ptr<Brush> Crimson() { return std::make_shared<SolidColorBrush>(0xFFDC143Cu); }
    static std::shared_ptr<Brush> Maroon() { return std::make_shared<SolidColorBrush>(0xFF800000u); }
    static std::shared_ptr<Brush> DarkRed() { return std::make_shared<SolidColorBrush>(0xFF8B0000u); }

    // Pinks
    static std::shared_ptr<Brush> Pink() { return std::make_shared<SolidColorBrush>(0xFFFFC0CBu); }
    static std::shared_ptr<Brush> HotPink() { return std::make_shared<SolidColorBrush>(0xFFFF69B4u); }
    static std::shared_ptr<Brush> DeepPink() { return std::make_shared<SolidColorBrush>(0xFFFF1493u); }
    static std::shared_ptr<Brush> PaleVioletRed() { return std::make_shared<SolidColorBrush>(0xFFDB7093u); }
    static std::shared_ptr<Brush> LightPink() { return std::make_shared<SolidColorBrush>(0xFFFFB6C1u); }

    // Oranges
    static std::shared_ptr<Brush> Orange() { return std::make_shared<SolidColorBrush>(0xFFFFA500u); }
    static std::shared_ptr<Brush> Orange50() { return std::make_shared<SolidColorBrush>(0xFFFFF3E0u); }
    static std::shared_ptr<Brush> Orange100() { return std::make_shared<SolidColorBrush>(0xFFFFE0B2u); }
    static std::shared_ptr<Brush> Orange200() { return std::make_shared<SolidColorBrush>(0xFFFFCC80u); }
    static std::shared_ptr<Brush> Orange300() { return std::make_shared<SolidColorBrush>(0xFFFFB74Du); }
    static std::shared_ptr<Brush> Orange400() { return std::make_shared<SolidColorBrush>(0xFFFFA726u); }
    static std::shared_ptr<Brush> Orange500() { return std::make_shared<SolidColorBrush>(0xFFFF9800u); }
    static std::shared_ptr<Brush> Orange600() { return std::make_shared<SolidColorBrush>(0xFFFB8C00u); }
    static std::shared_ptr<Brush> Orange700() { return std::make_shared<SolidColorBrush>(0xFFF57C00u); }
    static std::shared_ptr<Brush> Orange800() { return std::make_shared<SolidColorBrush>(0xFFEF6C00u); }
    static std::shared_ptr<Brush> Orange900() { return std::make_shared<SolidColorBrush>(0xFFE65100u); }
    static std::shared_ptr<Brush> Coral() { return std::make_shared<SolidColorBrush>(0xFFFF7F50u); }
    static std::shared_ptr<Brush> Tomato() { return std::make_shared<SolidColorBrush>(0xFFFF6347u); }
    static std::shared_ptr<Brush> DarkOrange() { return std::make_shared<SolidColorBrush>(0xFFFF8C00u); }
    static std::shared_ptr<Brush> PeachPuff() { return std::make_shared<SolidColorBrush>(0xFFFFDAB9u); }

    // Yellows
    static std::shared_ptr<Brush> Yellow() { return std::make_shared<SolidColorBrush>(0xFFFFFF00u); }
    static std::shared_ptr<Brush> Yellow50() { return std::make_shared<SolidColorBrush>(0xFFFFFDE7u); }
    static std::shared_ptr<Brush> Yellow100() { return std::make_shared<SolidColorBrush>(0xFFFFF9C4u); }
    static std::shared_ptr<Brush> Yellow200() { return std::make_shared<SolidColorBrush>(0xFFFFF59Du); }
    static std::shared_ptr<Brush> Yellow300() { return std::make_shared<SolidColorBrush>(0xFFFFF176u); }
    static std::shared_ptr<Brush> Yellow400() { return std::make_shared<SolidColorBrush>(0xFFFFEE58u); }
    static std::shared_ptr<Brush> Yellow500() { return std::make_shared<SolidColorBrush>(0xFFFFEB3Bu); }
    static std::shared_ptr<Brush> Yellow600() { return std::make_shared<SolidColorBrush>(0xFFFDD835u); }
    static std::shared_ptr<Brush> Yellow700() { return std::make_shared<SolidColorBrush>(0xFFFBC02Du); }
    static std::shared_ptr<Brush> Yellow800() { return std::make_shared<SolidColorBrush>(0xFFF9A825u); }
    static std::shared_ptr<Brush> Yellow900() { return std::make_shared<SolidColorBrush>(0xFFF57F17u); }
    static std::shared_ptr<Brush> Gold() { return std::make_shared<SolidColorBrush>(0xFFFFD700u); }
    static std::shared_ptr<Brush> Amber() { return std::make_shared<SolidColorBrush>(0xFFFFBF00u); }
    static std::shared_ptr<Brush> LightYellow() { return std::make_shared<SolidColorBrush>(0xFFFFFFE0u); }
    static std::shared_ptr<Brush> LemonChiffon() { return std::make_shared<SolidColorBrush>(0xFFFFFACDu); }

    //Green
    static std::shared_ptr<Brush> Green() { return std::make_shared<SolidColorBrush>(0xFF00FF00u); }
    static std::shared_ptr<Brush> Green50() { return std::make_shared<SolidColorBrush>(0xFFE8F5E9u); }
    static std::shared_ptr<Brush> Green100() { return std::make_shared<SolidColorBrush>(0xFFC8E6C9u); }
    static std::shared_ptr<Brush> Green200() { return std::make_shared<SolidColorBrush>(0xFFA5D6A7u); }
    static std::shared_ptr<Brush> Green300() { return std::make_shared<SolidColorBrush>(0xFF81C784u); }
    static std::shared_ptr<Brush> Green400() { return std::make_shared<SolidColorBrush>(0xFF66BB6Au); }
    static std::shared_ptr<Brush> Green500() { return std::make_shared<SolidColorBrush>(0xFF4CAF50u); }
    static std::shared_ptr<Brush> Green600() { return std::make_shared<SolidColorBrush>(0xFF43A047u); }
    static std::shared_ptr<Brush> Green700() { return std::make_shared<SolidColorBrush>(0xFF388E3Cu); }
    static std::shared_ptr<Brush> Green800() { return std::make_shared<SolidColorBrush>(0xFF2E7D32u); }
    static std::shared_ptr<Brush> Green900() { return std::make_shared<SolidColorBrush>(0xFF1B5E20u); }
    static std::shared_ptr<Brush> Lime() { return std::make_shared<SolidColorBrush>(0xFF00FF00u); }
    static std::shared_ptr<Brush> Lime50() { return std::make_shared<SolidColorBrush>(0xFFF9FBE7u); }
    static std::shared_ptr<Brush> Lime100() { return std::make_shared<SolidColorBrush>(0xFFF0F4C3u); }
    static std::shared_ptr<Brush> Lime200() { return std::make_shared<SolidColorBrush>(0xFFE6EE9Cu); }
    static std::shared_ptr<Brush> Lime300() { return std::make_shared<SolidColorBrush>(0xFFDCE775u); }
    static std::shared_ptr<Brush> Lime400() { return std::make_shared<SolidColorBrush>(0xFFD4E157u); }
    static std::shared_ptr<Brush> Lime500() { return std::make_shared<SolidColorBrush>(0xFFC0CA33u); }
    static std::shared_ptr<Brush> Lime600() { return std::make_shared<SolidColorBrush>(0xFFAFB42Bu); }
    static std::shared_ptr<Brush> Lime700() { return std::make_shared<SolidColorBrush>(0xFF9E9D24u); }
    static std::shared_ptr<Brush> Lime800() { return std::make_shared<SolidColorBrush>(0xFF827717u); }
    static std::shared_ptr<Brush> Lime900() { return std::make_shared<SolidColorBrush>(0xFF827717u); }
    static std::shared_ptr<Brush> ForestGreen() { return std::make_shared<SolidColorBrush>(0xFF228B22u); }
    static std::shared_ptr<Brush> DarkGreen() { return std::make_shared<SolidColorBrush>(0xFF006400u); }
    static std::shared_ptr<Brush> SeaGreen() { return std::make_shared<SolidColorBrush>(0xFF2E8B57u); }
    static std::shared_ptr<Brush> Olive() { return std::make_shared<SolidColorBrush>(0xFF808000u); }

    // Blues
    static std::shared_ptr<Brush> Blue() { return std::make_shared<SolidColorBrush>(0xFF0000FFu); }
    static std::shared_ptr<Brush> Blue50() { return std::make_shared<SolidColorBrush>(0xFFE3F2FDu); }
    static std::shared_ptr<Brush> Blue100() { return std::make_shared<SolidColorBrush>(0xFFBBDEFBu); }
    static std::shared_ptr<Brush> Blue200() { return std::make_shared<SolidColorBrush>(0xFF90CAF9u); }
    static std::shared_ptr<Brush> Blue300() { return std::make_shared<SolidColorBrush>(0xFF64B5F6u); }
    static std::shared_ptr<Brush> Blue400() { return std::make_shared<SolidColorBrush>(0xFF42A5F5u); }
    static std::shared_ptr<Brush> Blue500() { return std::make_shared<SolidColorBrush>(0xFF2196F3u); }
    static std::shared_ptr<Brush> Blue600() { return std::make_shared<SolidColorBrush>(0xFF1E88E5u); }
    static std::shared_ptr<Brush> Blue700() { return std::make_shared<SolidColorBrush>(0xFF1976D2u); }
    static std::shared_ptr<Brush> Blue800() { return std::make_shared<SolidColorBrush>(0xFF1565C0u); }
    static std::shared_ptr<Brush> Blue900() { return std::make_shared<SolidColorBrush>(0xFF0D47A1u); }
    static std::shared_ptr<Brush> SkyBlue() { return std::make_shared<SolidColorBrush>(0xFF87CEEBu); }
    static std::shared_ptr<Brush> LightBlue() { return std::make_shared<SolidColorBrush>(0xFFADD8E6u); }
    static std::shared_ptr<Brush> Navy() { return std::make_shared<SolidColorBrush>(0xFF000080u); }
    static std::shared_ptr<Brush> DarkBlue() { return std::make_shared<SolidColorBrush>(0xFF00008Bu); }
    static std::shared_ptr<Brush> RoyalBlue() { return std::make_shared<SolidColorBrush>(0xFF4169E1u); }
    static std::shared_ptr<Brush> CornflowerBlue() { return std::make_shared<SolidColorBrush>(0xFF6495EDu); }
    static std::shared_ptr<Brush> MidnightBlue() { return std::make_shared<SolidColorBrush>(0xFF191970u); }
    static std::shared_ptr<Brush> DodgerBlue() { return std::make_shared<SolidColorBrush>(0xFF1E90FFu); }
    static std::shared_ptr<Brush> DeepSkyBlue() { return std::make_shared<SolidColorBrush>(0xFF00BFFFu); }
    static std::shared_ptr<Brush> SteelBlue() { return std::make_shared<SolidColorBrush>(0xFF4682B4u); }

    // Purples
    static std::shared_ptr<Brush> Purple() { return std::make_shared<SolidColorBrush>(0xFF800080u); }
    static std::shared_ptr<Brush> Purple50() { return std::make_shared<SolidColorBrush>(0xFFF3E5F5u); }
    static std::shared_ptr<Brush> Purple100() { return std::make_shared<SolidColorBrush>(0xFFE1BEE7u); }
    static std::shared_ptr<Brush> Purple200() { return std::make_shared<SolidColorBrush>(0xFFCE93D8u); }
    static std::shared_ptr<Brush> Purple300() { return std::make_shared<SolidColorBrush>(0xFFBA68C8u); }
    static std::shared_ptr<Brush> Purple400() { return std::make_shared<SolidColorBrush>(0xFFAB47BCu); }
    static std::shared_ptr<Brush> Purple500() { return std::make_shared<SolidColorBrush>(0xFF9C27B0u); }
    static std::shared_ptr<Brush> Purple600() { return std::make_shared<SolidColorBrush>(0xFF8E24AAu); }
    static std::shared_ptr<Brush> Purple700() { return std::make_shared<SolidColorBrush>(0xFF7B1FA2u); }
    static std::shared_ptr<Brush> Purple800() { return std::make_shared<SolidColorBrush>(0xFF6A1B9Au); }
    static std::shared_ptr<Brush> Purple900() { return std::make_shared<SolidColorBrush>(0xFF4A148Cu); }
    static std::shared_ptr<Brush> Indigo() { return std::make_shared<SolidColorBrush>(0xFF4B0082u); }
    static std::shared_ptr<Brush> Indigo50() { return std::make_shared<SolidColorBrush>(0xFFE8EAF6u); }
    static std::shared_ptr<Brush> Indigo100() { return std::make_shared<SolidColorBrush>(0xFFC5CAE9u); }
    static std::shared_ptr<Brush> Indigo200() { return std::make_shared<SolidColorBrush>(0xFF9FA8DAu); }
    static std::shared_ptr<Brush> Indigo300() { return std::make_shared<SolidColorBrush>(0xFF7986CBu); }
    static std::shared_ptr<Brush> Indigo400() { return std::make_shared<SolidColorBrush>(0xFF5C6BC0u); }
    static std::shared_ptr<Brush> Indigo500() { return std::make_shared<SolidColorBrush>(0xFF3F51B5u); }
    static std::shared_ptr<Brush> Indigo600() { return std::make_shared<SolidColorBrush>(0xFF3949ABu); }
    static std::shared_ptr<Brush> Indigo700() { return std::make_shared<SolidColorBrush>(0xFF303F9Fu); }
    static std::shared_ptr<Brush> Indigo800() { return std::make_shared<SolidColorBrush>(0xFF283593u); }
    static std::shared_ptr<Brush> Indigo900() { return std::make_shared<SolidColorBrush>(0xFF1A237Eu); }
    static std::shared_ptr<Brush> Violet() { return std::make_shared<SolidColorBrush>(0xFFEE82EEu); }
    static std::shared_ptr<Brush> Orchid() { return std::make_shared<SolidColorBrush>(0xFFDA70D6u); }
    static std::shared_ptr<Brush> Plum() { return std::make_shared<SolidColorBrush>(0xFFDDA0DDu); }
    static std::shared_ptr<Brush> Magenta() { return std::make_shared<SolidColorBrush>(0xFFFF00FFu); }
    static std::shared_ptr<Brush> DarkMagenta() { return std::make_shared<SolidColorBrush>(0xFF8B008Bu); }
    static std::shared_ptr<Brush> RebeccaPurple() { return std::make_shared<SolidColorBrush>(0xFF663399u); }

    // Teals and Cyans
    static std::shared_ptr<Brush> Teal() { return std::make_shared<SolidColorBrush>(0xFF008080u); }
    static std::shared_ptr<Brush> Teal50() { return std::make_shared<SolidColorBrush>(0xFFE0F2F1u); }
    static std::shared_ptr<Brush> Teal100() { return std::make_shared<SolidColorBrush>(0xFFB2DFDBu); }
    static std::shared_ptr<Brush> Teal200() { return std::make_shared<SolidColorBrush>(0xFF80CBC4u); }
    static std::shared_ptr<Brush> Teal300() { return std::make_shared<SolidColorBrush>(0xFF4DB6ACu); }
    static std::shared_ptr<Brush> Teal400() { return std::make_shared<SolidColorBrush>(0xFF26A69Au); }
    static std::shared_ptr<Brush> Teal500() { return std::make_shared<SolidColorBrush>(0xFF009688u); }
    static std::shared_ptr<Brush> Teal600() { return std::make_shared<SolidColorBrush>(0xFF00897Bu); }
    static std::shared_ptr<Brush> Teal700() { return std::make_shared<SolidColorBrush>(0xFF00796Bu); }
    static std::shared_ptr<Brush> Teal800() { return std::make_shared<SolidColorBrush>(0xFF00695Cu); }
    static std::shared_ptr<Brush> Teal900() { return std::make_shared<SolidColorBrush>(0xFF004D40u); }
    static std::shared_ptr<Brush> Cyan() { return std::make_shared<SolidColorBrush>(0xFF00FFFFu); }
    static std::shared_ptr<Brush> Cyan50() { return std::make_shared<SolidColorBrush>(0xFFE0F7FAu); }
    static std::shared_ptr<Brush> Cyan100() { return std::make_shared<SolidColorBrush>(0xFFB2EBF2u); }
    static std::shared_ptr<Brush> Cyan200() { return std::make_shared<SolidColorBrush>(0xFF80DEEAu); }
    static std::shared_ptr<Brush> Cyan300() { return std::make_shared<SolidColorBrush>(0xFF4DD0E1u); }
    static std::shared_ptr<Brush> Cyan400() { return std::make_shared<SolidColorBrush>(0xFF26C6DAu); }
    static std::shared_ptr<Brush> Cyan500() { return std::make_shared<SolidColorBrush>(0xFF00BCD4u); }
    static std::shared_ptr<Brush> Cyan600() { return std::make_shared<SolidColorBrush>(0xFF00ACC1u); }
    static std::shared_ptr<Brush> Cyan700() { return std::make_shared<SolidColorBrush>(0xFF0097A7u); }
    static std::shared_ptr<Brush> Cyan800() { return std::make_shared<SolidColorBrush>(0xFF00838Fu); }
    static std::shared_ptr<Brush> Cyan900() { return std::make_shared<SolidColorBrush>(0xFF006064u); }
    static std::shared_ptr<Brush> DarkCyan() { return std::make_shared<SolidColorBrush>(0xFF008B8Bu); }
    static std::shared_ptr<Brush> Aqua() { return std::make_shared<SolidColorBrush>(0xFF00FFFFu); }
    static std::shared_ptr<Brush> Turquoise() { return std::make_shared<SolidColorBrush>(0xFF40E0D0u); }
    static std::shared_ptr<Brush> MediumTurquoise() { return std::make_shared<SolidColorBrush>(0xFF48D1CCu); }

    // Browns
    static std::shared_ptr<Brush> Brown() { return std::make_shared<SolidColorBrush>(0xFFA52A2Au); }
    static std::shared_ptr<Brush> Brown50() { return std::make_shared<SolidColorBrush>(0xFFEFEBE9u); }
    static std::shared_ptr<Brush> Brown100() { return std::make_shared<SolidColorBrush>(0xFFD7CCC8u); }
    static std::shared_ptr<Brush> Brown200() { return std::make_shared<SolidColorBrush>(0xFFBCAAA4u); }
    static std::shared_ptr<Brush> Brown300() { return std::make_shared<SolidColorBrush>(0xFFA1887Fu); }
    static std::shared_ptr<Brush> Brown400() { return std::make_shared<SolidColorBrush>(0xFF8D6E63u); }
    static std::shared_ptr<Brush> Brown500() { return std::make_shared<SolidColorBrush>(0xFF795548u); }
    static std::shared_ptr<Brush> Brown600() { return std::make_shared<SolidColorBrush>(0xFF6D4C41u); }
    static std::shared_ptr<Brush> Brown700() { return std::make_shared<SolidColorBrush>(0xFF5D4037u); }
    static std::shared_ptr<Brush> Brown800() { return std::make_shared<SolidColorBrush>(0xFF4E342Eu); }
    static std::shared_ptr<Brush> Brown900() { return std::make_shared<SolidColorBrush>(0xFF3E2723u); }
    static std::shared_ptr<Brush> SaddleBrown() { return std::make_shared<SolidColorBrush>(0xFF8B4513u); }
    static std::shared_ptr<Brush> Chocolate() { return std::make_shared<SolidColorBrush>(0xFFD2691Eu); }
    static std::shared_ptr<Brush> Sienna() { return std::make_shared<SolidColorBrush>(0xFFA0522Du); }
    static std::shared_ptr<Brush> Tan() { return std::make_shared<SolidColorBrush>(0xFFD2B48Cu); }
    static std::shared_ptr<Brush> Bisque() { return std::make_shared<SolidColorBrush>(0xFFFFE4C4u); }
    static std::shared_ptr<Brush> BlanchedAlmond() { return std::make_shared<SolidColorBrush>(0xFFFFEBCDu); }

    // Tailwind larping
    static std::shared_ptr<Brush> Slate50() { return std::make_shared<SolidColorBrush>(0xFFF8FAFCu); }
    static std::shared_ptr<Brush> Slate100() { return std::make_shared<SolidColorBrush>(0xFFF1F5F9u); }
    static std::shared_ptr<Brush> Slate200() { return std::make_shared<SolidColorBrush>(0xFFE2E8F0u); }
    static std::shared_ptr<Brush> Slate300() { return std::make_shared<SolidColorBrush>(0xFFCBD5E1u); }
    static std::shared_ptr<Brush> Slate400() { return std::make_shared<SolidColorBrush>(0xFF94A3B8u); }
    static std::shared_ptr<Brush> Slate500() { return std::make_shared<SolidColorBrush>(0xFF64748Bu); }
    static std::shared_ptr<Brush> Slate600() { return std::make_shared<SolidColorBrush>(0xFF475569u); }
    static std::shared_ptr<Brush> Slate700() { return std::make_shared<SolidColorBrush>(0xFF334155u); }
    static std::shared_ptr<Brush> Slate800() { return std::make_shared<SolidColorBrush>(0xFF1E293Bu); }
    static std::shared_ptr<Brush> Slate900() { return std::make_shared<SolidColorBrush>(0xFF0F172Au); }

    static std::shared_ptr<Brush> Rose50() { return std::make_shared<SolidColorBrush>(0xFFFFF1F2u); }
    static std::shared_ptr<Brush> Rose100() { return std::make_shared<SolidColorBrush>(0xFFFFE4E6u); }
    static std::shared_ptr<Brush> Rose200() { return std::make_shared<SolidColorBrush>(0xFFFECDD3u); }
    static std::shared_ptr<Brush> Rose300() { return std::make_shared<SolidColorBrush>(0xFFFDA4AFu); }
    static std::shared_ptr<Brush> Rose400() { return std::make_shared<SolidColorBrush>(0xFFFB7185u); }
    static std::shared_ptr<Brush> Rose500() { return std::make_shared<SolidColorBrush>(0xFFF43F5Eu); }
    static std::shared_ptr<Brush> Rose600() { return std::make_shared<SolidColorBrush>(0xFFE11D48u); }
    static std::shared_ptr<Brush> Rose700() { return std::make_shared<SolidColorBrush>(0xFFBE123Cu); }
    static std::shared_ptr<Brush> Rose800() { return std::make_shared<SolidColorBrush>(0xFF9F1239u); }
    static std::shared_ptr<Brush> Rose900() { return std::make_shared<SolidColorBrush>(0xFF881337u); }

    static std::shared_ptr<Brush> Emerald50() { return std::make_shared<SolidColorBrush>(0xFFECFDF5u); }
    static std::shared_ptr<Brush> Emerald100() { return std::make_shared<SolidColorBrush>(0xFFD1FAE5u); }
    static std::shared_ptr<Brush> Emerald200() { return std::make_shared<SolidColorBrush>(0xFFA7F3D0u); }
    static std::shared_ptr<Brush> Emerald300() { return std::make_shared<SolidColorBrush>(0xFF6EE7B7u); }
    static std::shared_ptr<Brush> Emerald400() { return std::make_shared<SolidColorBrush>(0xFF34D399u); }
    static std::shared_ptr<Brush> Emerald500() { return std::make_shared<SolidColorBrush>(0xFF10B981u); }
    static std::shared_ptr<Brush> Emerald600() { return std::make_shared<SolidColorBrush>(0xFF059669u); }
    static std::shared_ptr<Brush> Emerald700() { return std::make_shared<SolidColorBrush>(0xFF047857u); }
    static std::shared_ptr<Brush> Emerald800() { return std::make_shared<SolidColorBrush>(0xFF065F46u); }
    static std::shared_ptr<Brush> Emerald900() { return std::make_shared<SolidColorBrush>(0xFF064E3Bu); }

    static std::shared_ptr<Brush> Fuchsia50() { return std::make_shared<SolidColorBrush>(0xFFFDF4FFu); }
    static std::shared_ptr<Brush> Fuchsia100() { return std::make_shared<SolidColorBrush>(0xFFFAE8FFu); }
    static std::shared_ptr<Brush> Fuchsia200() { return std::make_shared<SolidColorBrush>(0xFFF5D0FEu); }
    static std::shared_ptr<Brush> Fuchsia300() { return std::make_shared<SolidColorBrush>(0xFFF0ABFCu); }
    static std::shared_ptr<Brush> Fuchsia400() { return std::make_shared<SolidColorBrush>(0xFFE879F9u); }
    static std::shared_ptr<Brush> Fuchsia500() { return std::make_shared<SolidColorBrush>(0xFFD946EFu); }
    static std::shared_ptr<Brush> Fuchsia600() { return std::make_shared<SolidColorBrush>(0xFFC026D3u); }
    static std::shared_ptr<Brush> Fuchsia700() { return std::make_shared<SolidColorBrush>(0xFFA21CAFu); }
    static std::shared_ptr<Brush> Fuchsia800() { return std::make_shared<SolidColorBrush>(0xFF86198Fu); }
    static std::shared_ptr<Brush> Fuchsia900() { return std::make_shared<SolidColorBrush>(0xFF701A75u); }
};

} 