#pragma once

#include <cstdint>

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
    static constexpr Color Black   = Color(0xFF000000u);
    static constexpr Color White   = Color(0xFFFFFFFFu);
    static constexpr Color Transparent = Color(0x00000000u);

    //Grays 
    static constexpr Color Gray100 = Color(0xFFF7FAFCu);
    static constexpr Color Gray200 = Color(0xFFEDF2F7u);
    static constexpr Color Gray300 = Color(0xFFE2E8F0u);
    static constexpr Color Gray400 = Color(0xFFCBD5E0u);
    static constexpr Color Gray500 = Color(0xFFA0AEC0u);
    static constexpr Color Gray600 = Color(0xFF718096u);
    static constexpr Color Gray700 = Color(0xFF4A5568u);
    static constexpr Color Gray800 = Color(0xFF2D3748u);
    static constexpr Color Gray900 = Color(0xFF1A202Cu);

    // Reds
    static constexpr Color Red       = Color(0xFFFF0000u);
    static constexpr Color Red50     = Color(0xFFFFF5F5u);
    static constexpr Color Red100    = Color(0xFFFFEBEEu);
    static constexpr Color Red200    = Color(0xFFFFCDD2u);
    static constexpr Color Red300    = Color(0xFFEF9A9Au);
    static constexpr Color Red400    = Color(0xFFEF5350u);
    static constexpr Color Red500    = Color(0xFFF44336u);
    static constexpr Color Red600    = Color(0xFFE53935u);
    static constexpr Color Red700    = Color(0xFFD32F2Fu);
    static constexpr Color Red800    = Color(0xFFC62828u);
    static constexpr Color Red900    = Color(0xFFB71C1Cu);
    static constexpr Color Crimson   = Color(0xFFDC143Cu);
    static constexpr Color Maroon    = Color(0xFF800000u);
    static constexpr Color DarkRed   = Color(0xFF8B0000u);

    // Pinks
    static constexpr Color Pink      = Color(0xFFFFC0CBu);
    static constexpr Color HotPink   = Color(0xFFFF69B4u);
    static constexpr Color DeepPink  = Color(0xFFFF1493u);
    static constexpr Color PaleVioletRed = Color(0xFFDB7093u);
    static constexpr Color LightPink = Color(0xFFFFB6C1u);

    // Oranges
    static constexpr Color Orange    = Color(0xFFFFA500u);
    static constexpr Color Orange50  = Color(0xFFFFF3E0u);
    static constexpr Color Orange100 = Color(0xFFFFE0B2u);
    static constexpr Color Orange200 = Color(0xFFFFCC80u);
    static constexpr Color Orange300 = Color(0xFFFFB74Du);
    static constexpr Color Orange400 = Color(0xFFFFA726u);
    static constexpr Color Orange500 = Color(0xFFFF9800u);
    static constexpr Color Orange600 = Color(0xFFFB8C00u);
    static constexpr Color Orange700 = Color(0xFFF57C00u);
    static constexpr Color Orange800 = Color(0xFFEF6C00u);
    static constexpr Color Orange900 = Color(0xFFE65100u);
    static constexpr Color Coral     = Color(0xFFFF7F50u);
    static constexpr Color Tomato    = Color(0xFFFF6347u);
    static constexpr Color DarkOrange = Color(0xFFFF8C00u);
    static constexpr Color PeachPuff = Color(0xFFFFDAB9u);

    // Yellows
    static constexpr Color Yellow    = Color(0xFFFFFF00u);
    static constexpr Color Yellow50  = Color(0xFFFFFDE7u);
    static constexpr Color Yellow100 = Color(0xFFFFF9C4u);
    static constexpr Color Yellow200 = Color(0xFFFFF59Du);
    static constexpr Color Yellow300 = Color(0xFFFFF176u);
    static constexpr Color Yellow400 = Color(0xFFFFEE58u);
    static constexpr Color Yellow500 = Color(0xFFFFEB3Bu);
    static constexpr Color Yellow600 = Color(0xFFFDD835u);
    static constexpr Color Yellow700 = Color(0xFFFBC02Du);
    static constexpr Color Yellow800 = Color(0xFFF9A825u);
    static constexpr Color Yellow900 = Color(0xFFF57F17u);
    static constexpr Color Gold      = Color(0xFFFFD700u);
    static constexpr Color Amber     = Color(0xFFFFBF00u);
    static constexpr Color LightYellow = Color(0xFFFFFFE0u);
    static constexpr Color LemonChiffon = Color(0xFFFFFACDu);

    //Green
    static constexpr Color Green     = Color(0xFF00FF00u);
    static constexpr Color Green50   = Color(0xFFE8F5E9u);
    static constexpr Color Green100  = Color(0xFFC8E6C9u);
    static constexpr Color Green200  = Color(0xFFA5D6A7u);
    static constexpr Color Green300  = Color(0xFF81C784u);
    static constexpr Color Green400  = Color(0xFF66BB6Au);
    static constexpr Color Green500  = Color(0xFF4CAF50u);
    static constexpr Color Green600  = Color(0xFF43A047u);
    static constexpr Color Green700  = Color(0xFF388E3Cu);
    static constexpr Color Green800  = Color(0xFF2E7D32u);
    static constexpr Color Green900  = Color(0xFF1B5E20u);
    static constexpr Color Lime      = Color(0xFF00FF00u);
    static constexpr Color Lime50    = Color(0xFFF9FBE7u);
    static constexpr Color Lime100   = Color(0xFFF0F4C3u);
    static constexpr Color Lime200   = Color(0xFFE6EE9Cu);
    static constexpr Color Lime300   = Color(0xFFDCE775u);
    static constexpr Color Lime400   = Color(0xFFD4E157u);
    static constexpr Color Lime500   = Color(0xFFC0CA33u);
    static constexpr Color Lime600   = Color(0xFFAFB42Bu);
    static constexpr Color Lime700   = Color(0xFF9E9D24u);
    static constexpr Color Lime800   = Color(0xFF827717u);
    static constexpr Color Lime900   = Color(0xFF827717u);
    static constexpr Color ForestGreen = Color(0xFF228B22u);
    static constexpr Color DarkGreen = Color(0xFF006400u);
    static constexpr Color SeaGreen  = Color(0xFF2E8B57u);
    static constexpr Color Olive     = Color(0xFF808000u);

    // Blues
    static constexpr Color Blue      = Color(0xFF0000FFu);
    static constexpr Color Blue50    = Color(0xFFE3F2FDu);
    static constexpr Color Blue100   = Color(0xFFBBDEFBu);
    static constexpr Color Blue200   = Color(0xFF90CAF9u);
    static constexpr Color Blue300   = Color(0xFF64B5F6u);
    static constexpr Color Blue400   = Color(0xFF42A5F5u);
    static constexpr Color Blue500   = Color(0xFF2196F3u);
    static constexpr Color Blue600   = Color(0xFF1E88E5u);
    static constexpr Color Blue700   = Color(0xFF1976D2u);
    static constexpr Color Blue800   = Color(0xFF1565C0u);
    static constexpr Color Blue900   = Color(0xFF0D47A1u);
    static constexpr Color SkyBlue   = Color(0xFF87CEEBu);
    static constexpr Color LightBlue = Color(0xFFADD8E6u);
    static constexpr Color Navy      = Color(0xFF000080u);
    static constexpr Color DarkBlue  = Color(0xFF00008Bu);
    static constexpr Color RoyalBlue = Color(0xFF4169E1u);
    static constexpr Color CornflowerBlue = Color(0xFF6495EDu);
    static constexpr Color MidnightBlue = Color(0xFF191970u);
    static constexpr Color DodgerBlue = Color(0xFF1E90FFu);
    static constexpr Color DeepSkyBlue = Color(0xFF00BFFFu);
    static constexpr Color SteelBlue = Color(0xFF4682B4u);

    // Purples
    static constexpr Color Purple    = Color(0xFF800080u);
    static constexpr Color Purple50  = Color(0xFFF3E5F5u);
    static constexpr Color Purple100 = Color(0xFFE1BEE7u);
    static constexpr Color Purple200 = Color(0xFFCE93D8u);
    static constexpr Color Purple300 = Color(0xFFBA68C8u);
    static constexpr Color Purple400 = Color(0xFFAB47BCu);
    static constexpr Color Purple500 = Color(0xFF9C27B0u);
    static constexpr Color Purple600 = Color(0xFF8E24AAu);
    static constexpr Color Purple700 = Color(0xFF7B1FA2u);
    static constexpr Color Purple800 = Color(0xFF6A1B9Au);
    static constexpr Color Purple900 = Color(0xFF4A148Cu);
    static constexpr Color Indigo    = Color(0xFF4B0082u);
    static constexpr Color Indigo50  = Color(0xFFE8EAF6u);
    static constexpr Color Indigo100 = Color(0xFFC5CAE9u);
    static constexpr Color Indigo200 = Color(0xFF9FA8DAu);
    static constexpr Color Indigo300 = Color(0xFF7986CBu);
    static constexpr Color Indigo400 = Color(0xFF5C6BC0u);
    static constexpr Color Indigo500 = Color(0xFF3F51B5u);
    static constexpr Color Indigo600 = Color(0xFF3949ABu);
    static constexpr Color Indigo700 = Color(0xFF303F9Fu);
    static constexpr Color Indigo800 = Color(0xFF283593u);
    static constexpr Color Indigo900 = Color(0xFF1A237Eu);
    static constexpr Color Violet    = Color(0xFFEE82EEu);
    static constexpr Color Orchid    = Color(0xFFDA70D6u);
    static constexpr Color Plum      = Color(0xFFDDA0DDu);
    static constexpr Color Magenta   = Color(0xFFFF00FFu);
    static constexpr Color DarkMagenta = Color(0xFF8B008Bu);
    static constexpr Color RebeccaPurple = Color(0xFF663399u);

    // Teals and Cyans
    static constexpr Color Teal      = Color(0xFF008080u);
    static constexpr Color Teal50    = Color(0xFFE0F2F1u);
    static constexpr Color Teal100   = Color(0xFFB2DFDBu);
    static constexpr Color Teal200   = Color(0xFF80CBC4u);
    static constexpr Color Teal300   = Color(0xFF4DB6ACu);
    static constexpr Color Teal400   = Color(0xFF26A69Au);
    static constexpr Color Teal500   = Color(0xFF009688u);
    static constexpr Color Teal600   = Color(0xFF00897Bu);
    static constexpr Color Teal700   = Color(0xFF00796Bu);
    static constexpr Color Teal800   = Color(0xFF00695Cu);
    static constexpr Color Teal900   = Color(0xFF004D40u);
    static constexpr Color Cyan      = Color(0xFF00FFFFu);
    static constexpr Color Cyan50    = Color(0xFFE0F7FAu);
    static constexpr Color Cyan100   = Color(0xFFB2EBF2u);
    static constexpr Color Cyan200   = Color(0xFF80DEEAu);
    static constexpr Color Cyan300   = Color(0xFF4DD0E1u);
    static constexpr Color Cyan400   = Color(0xFF26C6DAu);
    static constexpr Color Cyan500   = Color(0xFF00BCD4u);
    static constexpr Color Cyan600   = Color(0xFF00ACC1u);
    static constexpr Color Cyan700   = Color(0xFF0097A7u);
    static constexpr Color Cyan800   = Color(0xFF00838Fu);
    static constexpr Color Cyan900   = Color(0xFF006064u);
    static constexpr Color DarkCyan  = Color(0xFF008B8Bu);
    static constexpr Color Aqua      = Color(0xFF00FFFFu);
    static constexpr Color Turquoise = Color(0xFF40E0D0u);
    static constexpr Color MediumTurquoise = Color(0xFF48D1CCu);

    // Browns
    static constexpr Color Brown     = Color(0xFFA52A2Au);
    static constexpr Color Brown50   = Color(0xFFEFEBE9u);
    static constexpr Color Brown100  = Color(0xFFD7CCC8u);
    static constexpr Color Brown200  = Color(0xFFBCAAA4u);
    static constexpr Color Brown300  = Color(0xFFA1887Fu);
    static constexpr Color Brown400  = Color(0xFF8D6E63u);
    static constexpr Color Brown500  = Color(0xFF795548u);
    static constexpr Color Brown600  = Color(0xFF6D4C41u);
    static constexpr Color Brown700  = Color(0xFF5D4037u);
    static constexpr Color Brown800  = Color(0xFF4E342Eu);
    static constexpr Color Brown900  = Color(0xFF3E2723u);
    static constexpr Color SaddleBrown = Color(0xFF8B4513u);
    static constexpr Color Chocolate = Color(0xFFD2691Eu);
    static constexpr Color Sienna    = Color(0xFFA0522Du);
    static constexpr Color Tan       = Color(0xFFD2B48Cu);
    static constexpr Color Bisque    = Color(0xFFFFE4C4u);
    static constexpr Color BlanchedAlmond = Color(0xFFFFEBCDu);

    // Tailwind larping
    static constexpr Color Slate50   = Color(0xFFF8FAFCu);
    static constexpr Color Slate100  = Color(0xFFF1F5F9u);
    static constexpr Color Slate200  = Color(0xFFE2E8F0u);
    static constexpr Color Slate300  = Color(0xFFCBD5E1u);
    static constexpr Color Slate400  = Color(0xFF94A3B8u);
    static constexpr Color Slate500  = Color(0xFF64748Bu);
    static constexpr Color Slate600  = Color(0xFF475569u);
    static constexpr Color Slate700  = Color(0xFF334155u);
    static constexpr Color Slate800  = Color(0xFF1E293Bu);
    static constexpr Color Slate900  = Color(0xFF0F172Au);

    static constexpr Color Rose50    = Color(0xFFFFF1F2u);
    static constexpr Color Rose100   = Color(0xFFFFE4E6u);
    static constexpr Color Rose200   = Color(0xFFFECDD3u);
    static constexpr Color Rose300   = Color(0xFFFDA4AFu);
    static constexpr Color Rose400   = Color(0xFFFB7185u);
    static constexpr Color Rose500   = Color(0xFFF43F5Eu);
    static constexpr Color Rose600   = Color(0xFFE11D48u);
    static constexpr Color Rose700   = Color(0xFFBE123Cu);
    static constexpr Color Rose800   = Color(0xFF9F1239u);
    static constexpr Color Rose900   = Color(0xFF881337u);

    static constexpr Color Emerald50  = Color(0xFFECFDF5u);
    static constexpr Color Emerald100 = Color(0xFFD1FAE5u);
    static constexpr Color Emerald200 = Color(0xFFA7F3D0u);
    static constexpr Color Emerald300 = Color(0xFF6EE7B7u);
    static constexpr Color Emerald400 = Color(0xFF34D399u);
    static constexpr Color Emerald500 = Color(0xFF10B981u);
    static constexpr Color Emerald600 = Color(0xFF059669u);
    static constexpr Color Emerald700 = Color(0xFF047857u);
    static constexpr Color Emerald800 = Color(0xFF065F46u);
    static constexpr Color Emerald900 = Color(0xFF064E3Bu);

    static constexpr Color Fuchsia50  = Color(0xFFFDF4FFu);
    static constexpr Color Fuchsia100 = Color(0xFFFAE8FFu);
    static constexpr Color Fuchsia200 = Color(0xFFF5D0FEu);
    static constexpr Color Fuchsia300 = Color(0xFFF0ABFCu);
    static constexpr Color Fuchsia400 = Color(0xFFE879F9u);
    static constexpr Color Fuchsia500 = Color(0xFFD946EFu);
    static constexpr Color Fuchsia600 = Color(0xFFC026D3u);
    static constexpr Color Fuchsia700 = Color(0xFFA21CAFu);
    static constexpr Color Fuchsia800 = Color(0xFF86198Fu);
    static constexpr Color Fuchsia900 = Color(0xFF701A75u);
};

} 