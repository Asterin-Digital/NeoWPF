#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlEntityDecoder.h>

#include <string>
#include <string_view>

namespace NeoWPF::UI::Markup::Xaml {

std::string DecodeXmlEntities(std::string_view value)
{
    std::string decoded;
    decoded.reserve(value.size());

    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '&') {
            if (value.compare(i, 5, "&amp;") == 0) {
                decoded.push_back('&');
                i += 4;
                continue;
            }
            if (value.compare(i, 4, "&lt;") == 0) {
                decoded.push_back('<');
                i += 3;
                continue;
            }
            if (value.compare(i, 4, "&gt;") == 0) {
                decoded.push_back('>');
                i += 3;
                continue;
            }
            if (value.compare(i, 6, "&quot;") == 0) {
                decoded.push_back('"');
                i += 5;
                continue;
            }
            if (value.compare(i, 6, "&apos;") == 0) {
                decoded.push_back('\'');
                i += 5;
                continue;
            }
        }

        decoded.push_back(value[i]);
    }

    return decoded;
}

}
