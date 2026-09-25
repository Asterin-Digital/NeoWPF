#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlEntityDecoder.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>

#include <cctype>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

namespace {

struct ParserState {
    std::string_view Source;
    std::size_t Offset = 0;
    std::size_t Line = 1;
    std::size_t Column = 1;
};

[[nodiscard]] bool IsWhitespace(char ch) noexcept
{
    return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

void SkipWhitespace(ParserState& state)
{
    while (state.Offset < state.Source.size()) {
        const char ch = state.Source[state.Offset];
        if (!IsWhitespace(ch)) {
            break;
        }

        if (ch == '\n') {
            ++state.Line;
            state.Column = 1;
        } else {
            ++state.Column;
        }
        ++state.Offset;
    }
}

void Expect(ParserState& state, char expected)
{
    SkipWhitespace(state);
    if (state.Offset >= state.Source.size() || state.Source[state.Offset] != expected) {
        throw std::runtime_error("Expected '" + std::string(1, expected) + "' at line " +
            std::to_string(state.Line) + ", column " + std::to_string(state.Column));
    }
    ++state.Offset;
    ++state.Column;
}

std::string ParseName(ParserState& state)
{
    SkipWhitespace(state);
    const std::size_t start = state.Offset;
    const std::size_t startLine = state.Line;
    const std::size_t startColumn = state.Column;

    while (state.Offset < state.Source.size()) {
        const char ch = state.Source[state.Offset];
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == ':' || ch == '-' || ch == '.') {
            ++state.Offset;
            ++state.Column;
            continue;
        }
        break;
    }

    if (state.Offset == start) {
        throw std::runtime_error("Expected name at line " + std::to_string(startLine) + ", column " + std::to_string(startColumn));
    }

    return std::string(state.Source.substr(start, state.Offset - start));
}

std::string ParseAttributeValue(ParserState& state)
{
    SkipWhitespace(state);
    Expect(state, '"');

    std::string value;
    while (state.Offset < state.Source.size()) {
        const char ch = state.Source[state.Offset];
        if (ch == '"') {
            ++state.Offset;
            ++state.Column;
            return DecodeXmlEntities(value);
        }

        if (ch == '\n') {
            ++state.Line;
            state.Column = 1;
        } else {
            ++state.Column;
        }

        value.push_back(ch);
        ++state.Offset;
    }

    throw std::runtime_error("Unterminated attribute value at line " + std::to_string(state.Line) + ", column " + std::to_string(state.Column));
}

struct ParsedAttributes {
    std::unordered_map<std::string, std::string> Values;
    std::string XamlName;
    std::string XamlKey;
    std::string XamlClass;

    std::string* FindSpecialAttribute(std::string_view key)
    {
        const std::array<std::pair<std::string_view, std::string*>, 3> specialAttributes = {{
            {"x:Name", &XamlName},
            {"x:Key", &XamlKey},
            {"x:Class", &XamlClass},
        }};

        for (const auto& [name, value] : specialAttributes) {
            if (name == key) {
                return value;
            }
        }
        return nullptr;
    }
};

ParsedAttributes ParseAttributes(ParserState& state)
{
    ParsedAttributes attributes;
    SkipWhitespace(state);

    while (state.Offset < state.Source.size()) {
        const char ch = state.Source[state.Offset];
        if (ch == '>') {
            break;
        }
        if (ch == '/' && state.Offset + 1 < state.Source.size() && state.Source[state.Offset + 1] == '>') {
            break;
        }

        const std::string key = ParseName(state);
        SkipWhitespace(state);
        Expect(state, '=');
        SkipWhitespace(state);
        const std::string value = ParseAttributeValue(state);
        if (auto* specialAttribute = attributes.FindSpecialAttribute(key)) {
            *specialAttribute = value;
        } else {
            attributes.Values[key] = value;
        }
        SkipWhitespace(state);
    }

    return attributes;
}

XamlNode ParseNode(ParserState& state)
{
    SkipWhitespace(state);
    if (state.Offset >= state.Source.size()) {
        throw std::runtime_error("Unexpected end of XAML input");
    }

    if (state.Source[state.Offset] == '<') {
        ++state.Offset;
        ++state.Column;

        if (state.Offset < state.Source.size() && state.Source[state.Offset] == '!') {
            if (state.Source.compare(state.Offset, 2, "--") == 0) {
                state.Offset += 2;
                state.Column += 2;
                while (state.Offset + 1 < state.Source.size()) {
                    if (state.Source[state.Offset] == '-' && state.Source[state.Offset + 1] == '-') {
                        state.Offset += 2;
                        state.Column += 2;
                        if (state.Offset < state.Source.size() && state.Source[state.Offset] == '>') {
                            ++state.Offset;
                            ++state.Column;
                            return ParseNode(state);
                        }
                    }
                    if (state.Source[state.Offset] == '\n') {
                        ++state.Line;
                        state.Column = 1;
                    } else {
                        ++state.Column;
                    }
                    ++state.Offset;
                }
                throw std::runtime_error("Unterminated comment at line " + std::to_string(state.Line) + ", column " + std::to_string(state.Column));
            }
            while (state.Offset < state.Source.size() && state.Source[state.Offset] != '>') {
                if (state.Source[state.Offset] == '\n') {
                    ++state.Line;
                    state.Column = 1;
                } else {
                    ++state.Column;
                }
                ++state.Offset;
            }
            if (state.Offset < state.Source.size() && state.Source[state.Offset] == '>') {
                ++state.Offset;
                ++state.Column;
                return ParseNode(state);
            }
        }

        if (state.Offset < state.Source.size() && state.Source[state.Offset] == '/') {
            throw std::runtime_error("Unexpected closing tag at line " + std::to_string(state.Line) + ", column " + std::to_string(state.Column));
        }

        const std::size_t nodeLine = state.Line;
        const std::size_t nodeColumn = state.Column;
        XamlNode node;
        node.Line = nodeLine;
        node.Column = nodeColumn;
        node.Name = ParseName(state);

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Info,
            "[XamlParser] Parsing element '%s' at line %zu", node.Name.c_str(), nodeLine);

        const auto attributes = ParseAttributes(state);
        node.Attributes = attributes.Values;
        node.XamlName = attributes.XamlName;
        node.XamlKey = attributes.XamlKey;
        node.XamlClass = attributes.XamlClass;

        if (state.Offset < state.Source.size() && state.Source[state.Offset] == '/') {
            ++state.Offset;
            ++state.Column;
            Expect(state, '>');
            return node;
        }

        Expect(state, '>');

        while (state.Offset < state.Source.size()) {
            SkipWhitespace(state);
            if (state.Offset >= state.Source.size()) {
                break;
            }
            if (state.Source[state.Offset] == '<') {
                if (state.Offset + 1 < state.Source.size() && state.Source[state.Offset + 1] == '/') {
                    state.Offset += 2;
                    state.Column += 2;
                    const std::string closingTag = ParseName(state);
                    if (closingTag != node.Name) {
                        throw std::runtime_error("Mismatched closing tag '</" + closingTag + ">' for node '<" + node.Name + ">'");
                    }
                    SkipWhitespace(state);
                    Expect(state, '>');
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Info,
                        "[XamlParser] Closed element '%s' with %zu children", node.Name.c_str(), node.Children.size());
                    return node;
                }

                XamlNode child = ParseNode(state);
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Info,
                    "[XamlParser] Added child '%s' to parent '%s'", child.Name.c_str(), node.Name.c_str());
                node.Children.push_back(child);
                continue;
            }

            const std::size_t textStart = state.Offset;
            const std::size_t textLine = state.Line;
            const std::size_t textColumn = state.Column;
            while (state.Offset < state.Source.size() && state.Source[state.Offset] != '<') {
                if (state.Source[state.Offset] == '\n') {
                    ++state.Line;
                    state.Column = 1;
                } else {
                    ++state.Column;
                }
                ++state.Offset;
            }

            const std::string text = DecodeXmlEntities(std::string(state.Source.substr(textStart, state.Offset - textStart)));
            if (!text.empty()) {
                node.TextContent += text;

                if (!node.Children.empty() && node.Children.back().IsTextNode()) {
                    node.Children.back().TextContent += text;
                } else {
                    XamlNode textNode;
                    textNode.Line = textLine;
                    textNode.Column = textColumn;
                    textNode.TextContent = text;
                    node.Children.push_back(textNode);
                }
            }
        }

        throw std::runtime_error("Unterminated element '<" + node.Name + ">' at line " + std::to_string(node.Line) + ", column " + std::to_string(node.Column));
    }

    throw std::runtime_error("Unexpected token at line " + std::to_string(state.Line) + ", column " + std::to_string(state.Column));
}

}

std::vector<XamlNode> ParseXaml(const std::string& source)
{
    NEO_WPF_ACTION("Xaml.Parse");

    ParserState state{source};
    std::vector<XamlNode> roots;

    while (state.Offset < source.size()) {
        SkipWhitespace(state);
        if (state.Offset >= source.size()) {
            break;
        }
        if (source[state.Offset] == '<' && state.Offset + 1 < source.size() && source[state.Offset + 1] == '?') {

            while (state.Offset < source.size() && !(source[state.Offset] == '?' && state.Offset + 1 < source.size() && source[state.Offset + 1] == '>')) {
                if (source[state.Offset] == '\n') {
                    ++state.Line;
                    state.Column = 1;
                } else {
                    ++state.Column;
                }
                ++state.Offset;
            }
            if (state.Offset + 1 < source.size()) {
                state.Offset += 2;
                state.Column += 2;
            }
            continue;
        }
        roots.push_back(ParseNode(state));
    }

    return roots;
}

}
