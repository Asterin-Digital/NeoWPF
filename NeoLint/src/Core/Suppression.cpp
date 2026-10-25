#include <NeoLint/Core/Suppression.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace neolint
{
namespace
{
std::string Trim(std::string value)
{
    const auto first = std::find_if_not(value.begin(), value.end(),
                                        [](unsigned char character) {
                                            return std::isspace(character) != 0;
                                        });
    const auto last = std::find_if_not(value.rbegin(), value.rend(),
                                       [](unsigned char character) {
                                           return std::isspace(character) != 0;
                                       }).base();
    if (first >= last) return {};
    return {first, last};
}

bool HasDirective(std::string_view line, std::string_view marker,
                  std::string_view ruleId)
{
    const auto markerPosition = line.find(marker);
    if (markerPosition == std::string_view::npos) return false;
    const auto commentPosition = line.rfind("//", markerPosition);
    if (commentPosition == std::string_view::npos) return false;
    const auto declaration = Trim(std::string(line.substr(markerPosition + marker.size())));
    const auto separator = declaration.find(':');
    if (separator == std::string::npos) return false;
    return Trim(declaration.substr(0, separator)) == ruleId &&
        !Trim(declaration.substr(separator + 1)).empty();
}
}

Suppression::Suppression(const clang::SourceManager& sourceManager) noexcept
    : sourceManager_(sourceManager)
{
}

bool Suppression::IsSuppressed(std::string_view ruleId, clang::SourceLocation location) const
{
    const auto decomposed = sourceManager_.getDecomposedLoc(location);
    const auto buffer = sourceManager_.getBufferData(decomposed.first);
    const std::string_view source(buffer.data(), buffer.size());
    const auto currentOffset = std::min<std::size_t>(decomposed.second, source.size());
    const auto lineNumber = static_cast<std::size_t>(
        std::count(source.begin(), source.begin() + currentOffset, '\n')) + 1;
    return IsSuppressed(ruleId, source, lineNumber);
}

bool Suppression::IsSuppressed(std::string_view ruleId, std::string_view source,
                               std::size_t lineNumber)
{
    if (lineNumber == 0) return false;
    auto currentLineStart = std::size_t{0};
    for (auto line = std::size_t{1}; line < lineNumber && currentLineStart < source.size(); ++line)
    {
        const auto lineEnd = source.find('\n', currentLineStart);
        currentLineStart = lineEnd == std::string_view::npos ? source.size() : lineEnd + 1;
    }
    if (currentLineStart > source.size()) return false;
    auto currentLineEnd = source.find('\n', currentLineStart);
    if (currentLineEnd == std::string_view::npos) currentLineEnd = source.size();
    const auto currentLine = source.substr(currentLineStart, currentLineEnd - currentLineStart);
    if (HasDirective(currentLine, "neolint-disable-line", ruleId)) return true;

    if (currentLineStart > 0)
    {
        auto previousLineEnd = currentLineStart - 1;
        if (previousLineEnd > 0 && source[previousLineEnd - 1] == '\r') --previousLineEnd;
        auto previousLineStart = previousLineEnd;
        while (previousLineStart > 0 && source[previousLineStart - 1] != '\n')
        {
            --previousLineStart;
        }
        const auto previousLine = source.substr(previousLineStart,
                                                previousLineEnd - previousLineStart);
        if (HasDirective(previousLine, "neolint-disable-next-line", ruleId)) return true;
    }

    auto lineStart = std::size_t{0};
    while (lineStart < source.size())
    {
        auto lineEnd = source.find('\n', lineStart);
        if (lineEnd == std::string_view::npos) lineEnd = source.size();
        if (HasDirective(source.substr(lineStart, lineEnd - lineStart),
                         "neolint-disable-file", ruleId)) return true;
        lineStart = lineEnd + 1;
    }
    return false;
}
}
