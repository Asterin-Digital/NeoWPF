#include <NeoWPF/UI/Text/TextFlow.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <algorithm>
#include <cwctype>

/////////////
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
/////////////

namespace NeoWPF::UI::Text {

namespace {

struct LogicalLine {
    std::wstring text;
    std::size_t startOffset = 0;
};

std::vector<LogicalLine> SplitLines(const std::wstring& text) {
    std::vector<LogicalLine> lines;
    std::size_t lineStart = 0;
    while (lineStart <= text.size()) {
        const std::size_t lineBreak = text.find(L'\n', lineStart);
        const std::size_t lineEnd = lineBreak == std::wstring::npos ? text.size() : lineBreak;
        lines.push_back({text.substr(lineStart, lineEnd - lineStart), lineStart});
        if (lineBreak == std::wstring::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }
    return lines;
}

bool FitsWithin(const std::wstring& text, int maxWidth, int fontSize, const std::wstring& fontFamily) {
    if (maxWidth <= 0 || text.empty()) {
        return true;
    }
    return NeoWPF::Rendering::Text::MeasureText(text, fontSize, fontFamily, 0, 0).width <= maxWidth;
}

struct Token {
    std::wstring text;
    std::size_t startOffset = 0;
    bool isWhitespace = false;
};

std::vector<Token> Tokenize(const std::wstring& lineText, std::size_t lineStartOffset) {
    std::vector<Token> tokens;
    std::size_t i = 0;
    while (i < lineText.size()) {
        const bool isWhitespace = std::iswspace(static_cast<wint_t>(lineText[i])) != 0;
        std::size_t j = i;
        while (j < lineText.size() && (std::iswspace(static_cast<wint_t>(lineText[j])) != 0) == isWhitespace) {
            ++j;
        }
        tokens.push_back({lineText.substr(i, j - i), lineStartOffset + i, isWhitespace});
        i = j;
    }
    return tokens;
}

struct WrappedSegment {
    std::wstring text;
    std::size_t startOffset = 0;
    std::size_t endOffset = 0;
};

void BreakWordByCharacter(const std::wstring& word, std::size_t wordStart, int availableWidth,
                          int fontSize, const std::wstring& fontFamily,
                          std::vector<WrappedSegment>& out) {
    std::wstring piece;
    std::size_t pieceStart = wordStart;
    for (std::size_t k = 0; k < word.size(); ++k) {
        const std::wstring candidate = piece + word[k];
        if (piece.empty() || FitsWithin(candidate, availableWidth, fontSize, fontFamily)) {
            piece = candidate;
            continue;
        }

        out.push_back({piece, pieceStart, pieceStart + piece.size()});
        pieceStart += piece.size();
        piece = std::wstring(1, word[k]);
    }

    if (!piece.empty()) {
        out.push_back({piece, pieceStart, pieceStart + piece.size()});
    }
}

std::vector<WrappedSegment> WrapLine(const std::wstring& lineText, std::size_t lineStartOffset,
                                    int availableWidth, int fontSize, const std::wstring& fontFamily,
                                    TextWrapping wrapping) {
    std::vector<WrappedSegment> result;
    if (lineText.empty() || availableWidth <= 0 || wrapping == TextWrapping::NoWrap) {
        result.push_back({lineText, lineStartOffset, lineStartOffset + lineText.size()});
        return result;
    }

    const auto tokens = Tokenize(lineText, lineStartOffset);
    std::wstring currentLine;
    std::size_t currentLineStart = lineStartOffset;
    std::size_t currentLineEnd = lineStartOffset;

    auto flush = [&]() {
        if (!currentLine.empty()) {
            result.push_back({currentLine, currentLineStart, currentLineEnd});
        }
        currentLine.clear();
    };

    for (const auto& token : tokens) {
        const std::wstring candidate = currentLine + token.text;
        if (FitsWithin(candidate, availableWidth, fontSize, fontFamily)) {
            currentLine = candidate;
            currentLineEnd = token.startOffset + token.text.size();
            continue;
        }

        if (token.isWhitespace) {
            flush();
            currentLineStart = token.startOffset + token.text.size();
            currentLineEnd = currentLineStart;
            continue;
        }

        if (FitsWithin(token.text, availableWidth, fontSize, fontFamily)) {
            flush();
            currentLine = token.text;
            currentLineStart = token.startOffset;
            currentLineEnd = token.startOffset + token.text.size();
            continue;
        }

        flush();
        if (wrapping == TextWrapping::Wrap) {
            BreakWordByCharacter(token.text, token.startOffset, availableWidth, fontSize, fontFamily, result);
            currentLineStart = token.startOffset + token.text.size();
            currentLineEnd = currentLineStart;
        }
        else {
            result.push_back({token.text, token.startOffset, token.startOffset + token.text.size()});
            currentLineStart = token.startOffset + token.text.size();
            currentLineEnd = currentLineStart;
        }
    }

    flush();
    if (result.empty()) {
        result.push_back({std::wstring(), lineStartOffset, lineStartOffset});
    }
    return result;
}

} // namespace

int MeasureLineHeight(const std::wstring& /*text*/, int fontSize, const std::wstring& fontFamily) {
    const int effectiveFontSize = fontSize > 0 ? fontSize : 16;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", effectiveFontSize, fontFamily, 0, 0);
    return std::max(baseMetrics.height, effectiveFontSize);
}

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                    int availableWidth, TextWrapping wrapping) {
    FlowResult flow;
    const auto logicalLines = SplitLines(text);

    for (const auto& logical : logicalLines) {
        if (logical.text.empty() && logical.startOffset >= text.size()) {
            const int lineHeight = std::max(1, MeasureLineHeight(L"W", fontSize, fontFamily));
            flow.lines.push_back({std::wstring(), logical.startOffset, logical.startOffset, 0, lineHeight, lineHeight});
            flow.contentHeight += lineHeight;
            continue;
        }

        const bool shouldWrap = wrapping != TextWrapping::NoWrap && availableWidth > 0;
        const int constraint = shouldWrap ? availableWidth : 0;
        const auto segments = shouldWrap
            ? WrapLine(logical.text, logical.startOffset, constraint, fontSize, fontFamily, wrapping)
            : std::vector<WrappedSegment>{{logical.text, logical.startOffset, logical.startOffset + logical.text.size()}};

        for (const auto& seg : segments) {
            const auto metrics = NeoWPF::Rendering::Text::MeasureText(
                seg.text.empty() ? L"W" : seg.text, fontSize, fontFamily, 0, 0);
            const int lineHeight = std::max(1, MeasureLineHeight(seg.text, fontSize, fontFamily));

            flow.lines.push_back({seg.text, seg.startOffset, seg.endOffset, metrics.width, metrics.height, lineHeight});
            flow.contentWidth = std::max(flow.contentWidth, metrics.width);
            flow.contentHeight += lineHeight;
        }
    }

    return flow;
}

}
