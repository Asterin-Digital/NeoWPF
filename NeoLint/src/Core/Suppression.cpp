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
}

Suppression::Suppression(const clang::SourceManager& sourceManager) noexcept
    : sourceManager_(sourceManager)
{
}

bool Suppression::IsSuppressed(std::string_view ruleId, clang::SourceLocation location) const
{
    const auto presumed = sourceManager_.getPresumedLoc(location);
    if (presumed.isInvalid() || presumed.getLine() <= 1) return false;

    const auto decomposed = sourceManager_.getDecomposedLoc(location);
    const auto buffer = sourceManager_.getBufferData(decomposed.first);
    const auto currentOffset = decomposed.second;
    if (currentOffset == 0 || currentOffset > buffer.size()) return false;

    auto previousLineEnd = currentOffset;
    while (previousLineEnd > 0 && buffer[previousLineEnd - 1] != '\n') --previousLineEnd;
    if (previousLineEnd == 0) return false;
    --previousLineEnd;

    auto previousLineStart = previousLineEnd;
    while (previousLineStart > 0 && buffer[previousLineStart - 1] != '\n') --previousLineStart;

    const auto previousLine = sourceManager_.getLocForStartOfFile(decomposed.first)
        .getLocWithOffset(static_cast<int>(previousLineStart));
    const auto* lineStart = sourceManager_.getCharacterData(previousLine);
    const auto* lineEnd = lineStart;
    while (*lineEnd != '\0' && *lineEnd != '\r' && *lineEnd != '\n') ++lineEnd;

    const std::string marker = "neolint-disable-next-line";
    const std::string line(lineStart, lineEnd);
    const auto markerPosition = line.find(marker);
    if (markerPosition == std::string::npos) return false;

    const auto declaration = Trim(line.substr(markerPosition + marker.size()));
    const auto separator = declaration.find(':');
    if (separator == std::string::npos) return false;

    const auto declaredRule = Trim(declaration.substr(0, separator));
    const auto reason = Trim(declaration.substr(separator + 1));
    return declaredRule == ruleId && !reason.empty();
}
}
