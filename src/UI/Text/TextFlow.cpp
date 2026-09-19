#include <NeoWPF/UI/Text/TextFlow.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <algorithm>
#include <cwctype>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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

bool FitsWithin(const std::wstring& text, double maxWidth, int fontSize, const std::wstring& fontFamily,
                int fontWeight, NeoWPF::UI::FontStyle fontStyle) {
    if (maxWidth <= 0.0 || text.empty()) {
        return true;
    }
    return NeoWPF::Rendering::Text::MeasureText(text, fontSize, fontFamily, 0, 0, fontWeight, fontStyle).width <= maxWidth;
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

void BreakWordByCharacter(const std::wstring& word, std::size_t wordStart, double availableWidth,
                          int fontSize, const std::wstring& fontFamily, int fontWeight,
                          NeoWPF::UI::FontStyle fontStyle,
                          std::vector<WrappedSegment>& out) {
    std::wstring piece;
    std::size_t pieceStart = wordStart;
    for (std::size_t k = 0; k < word.size(); ++k) {
        const std::wstring candidate = piece + word[k];
        if (piece.empty() || FitsWithin(candidate, availableWidth, fontSize, fontFamily, fontWeight, fontStyle)) {
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
                                    double availableWidth, int fontSize, const std::wstring& fontFamily,
                                    TextWrapping wrapping, int fontWeight,
                                    NeoWPF::UI::FontStyle fontStyle) {
    std::vector<WrappedSegment> result;
    if (lineText.empty() || availableWidth <= 0 || wrapping == TextWrapping::NoWrap) {
        WrappedSegment segment;
        segment.text = lineText;
        segment.startOffset = lineStartOffset;
        segment.endOffset = lineStartOffset + lineText.size();
        result.push_back(segment);
        return result;
    }

    const auto tokens = Tokenize(lineText, lineStartOffset);
    std::wstring currentLine;
    std::size_t currentLineStart = lineStartOffset;
    std::size_t currentLineEnd = lineStartOffset;

    auto flush = [&]() {
        if (!currentLine.empty()) {
            WrappedSegment segment;
            segment.text = currentLine;
            segment.startOffset = currentLineStart;
            segment.endOffset = currentLineEnd;
            result.push_back(segment);
        }
        currentLine.clear();
    };

    for (const auto& token : tokens) {
        const std::wstring candidate = currentLine + token.text;
        if (FitsWithin(candidate, availableWidth, fontSize, fontFamily, fontWeight, fontStyle)) {
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

        if (FitsWithin(token.text, availableWidth, fontSize, fontFamily, fontWeight, fontStyle)) {
            flush();
            currentLine = token.text;
            currentLineStart = token.startOffset;
            currentLineEnd = token.startOffset + token.text.size();
            continue;
        }

        flush();
        if (wrapping == TextWrapping::Wrap) {
            BreakWordByCharacter(token.text, token.startOffset, availableWidth, fontSize, fontFamily, fontWeight, fontStyle, result);
            currentLineStart = token.startOffset + token.text.size();
            currentLineEnd = currentLineStart;
        }
        else {
            WrappedSegment segment;
            segment.text = token.text;
            segment.startOffset = token.startOffset;
            segment.endOffset = token.startOffset + token.text.size();
            result.push_back(segment);
            currentLineStart = token.startOffset + token.text.size();
            currentLineEnd = currentLineStart;
        }
    }

    flush();
    if (result.empty()) {
        WrappedSegment segment;
        segment.text = std::wstring();
        segment.startOffset = lineStartOffset;
        segment.endOffset = lineStartOffset;
        result.push_back(segment);
    }
    return result;
}

}

int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily) {
    return MeasureLineHeight(text, fontSize, fontFamily, 400, NeoWPF::UI::FontStyle::Normal);
}

int MeasureLineHeight(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                     int fontWeight, NeoWPF::UI::FontStyle fontStyle) {
    const int effectiveFontSize = fontSize > 0 ? fontSize : 16;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", effectiveFontSize, fontFamily, 0, 0, fontWeight, fontStyle);
    const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(
        text.empty() ? L"W" : text, effectiveFontSize, fontFamily, 0, 0, fontWeight, fontStyle);
    return std::max(baseMetrics.height, std::max(lineMetrics.height, effectiveFontSize));
}

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                    int availableWidth, TextWrapping wrapping) {
    return BreakText(text, fontSize, fontFamily, availableWidth, wrapping, 400, NeoWPF::UI::FontStyle::Normal);
}

FlowResult BreakText(const std::wstring& text, int fontSize, const std::wstring& fontFamily,
                    double availableWidth, TextWrapping wrapping, int fontWeight,
                    NeoWPF::UI::FontStyle fontStyle) {
    FlowResult flow;
    const int effectiveFontSize = fontSize > 0 ? fontSize : 16;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(
        L"W", effectiveFontSize, fontFamily, 0, 0, fontWeight, fontStyle);
    double baseLineHeight = baseMetrics.height;
    if (effectiveFontSize > baseLineHeight) {
        baseLineHeight = effectiveFontSize;
    }
    const auto logicalLines = SplitLines(text);

    for (const auto& logical : logicalLines) {
        if (logical.text.empty() && logical.startOffset >= text.size()) {
            const double lineHeight = std::max(1.0, baseLineHeight);
            FlowLine emptyLine;
            emptyLine.text = std::wstring();
            emptyLine.charStart = logical.startOffset;
            emptyLine.charEnd = logical.startOffset;
            emptyLine.width = 0.0;
            emptyLine.height = lineHeight;
            emptyLine.lineHeight = lineHeight;
            emptyLine.inkTop = 0.0;
            flow.lines.push_back(emptyLine);
            flow.contentHeight += lineHeight;
            continue;
        }

        const bool shouldWrap = wrapping != TextWrapping::NoWrap && availableWidth > 0.0;
        const double constraint = shouldWrap ? availableWidth : 0.0;
        const auto segments = shouldWrap
            ? WrapLine(logical.text, logical.startOffset, constraint, fontSize, fontFamily, wrapping, fontWeight, fontStyle)
            : std::vector<WrappedSegment>{{logical.text, logical.startOffset, logical.startOffset + logical.text.size()}};

        for (const auto& seg : segments) {
            const auto metrics = NeoWPF::Rendering::Text::MeasureText(
                seg.text.empty() ? L"W" : seg.text, effectiveFontSize, fontFamily, 0, 0, fontWeight, fontStyle);
            double lineHeight = baseLineHeight;
            if (metrics.height > lineHeight) {
                lineHeight = metrics.height;
            }
            if (lineHeight < 1.0) {
                lineHeight = 1.0;
            }

            FlowLine line;
            line.text = seg.text;
            line.charStart = seg.startOffset;
            line.charEnd = seg.endOffset;
            line.width = metrics.width;
            line.height = metrics.height;
            line.lineHeight = lineHeight;
            line.inkTop = 0.0;
            flow.lines.push_back(line);
            if (metrics.width > flow.contentWidth) {
                flow.contentWidth = metrics.width;
            }
            flow.contentHeight += lineHeight;
        }
    }

    return flow;
}

}
