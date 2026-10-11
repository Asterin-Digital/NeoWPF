#include <NeoWPF/Diagnostics/TextDumper.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <sstream>
#include <iomanip>

namespace NeoWPF::Diagnostics {

namespace {

std::string FormatArgb(uint32_t argb)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << argb;
    return oss.str();
}

std::string FormatBrush(const std::shared_ptr<NeoWPF::UI::Brush>& brush)
{
    if (!brush) return "null";
    if (const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush)) {
        return FormatArgb(solidBrush->GetArgb());
    }
    return typeid(*brush).name();
}

std::string SafeText(const std::wstring& text)
{
    std::string out;
    out.reserve(text.size());
    for (wchar_t ch : text) {
        if (ch == '\\') out += "\\\\";
        else if (ch == '\n') out += "\\n";
        else if (ch == '\r') out += "\\r";
        else if (ch == '\t') out += "\\t";
        else if (ch >= 32 && ch < 127) out += static_cast<char>(ch);
        else {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04X", static_cast<unsigned int>(ch));
            out += buf;
        }
    }
    return out;
}

}

void TextDumper::Dump(NeoWPF::UI::Visual* root, const TextDumpOptions& options)
{
    if (!root) {
        LogWarn("TextDumper::Dump - root is null");
        return;
    }

    LogInfo("=== Text Dump ===");
    DumpRecursive(root, options, 0);
    LogInfo("=== End Text Dump ===");
}

void TextDumper::DumpRecursive(NeoWPF::UI::Visual* visual, const TextDumpOptions& options, int depth)
{
    if (!visual) return;

    auto* textBlock = dynamic_cast<NeoWPF::UI::TextBlock*>(visual);
    if (textBlock) {
        DumpTextBlock(textBlock, options, depth);
    }

    auto* textBox = dynamic_cast<NeoWPF::UI::TextBox*>(visual);
    if (textBox) {
        DumpTextBox(textBox, options, depth);
    }

    auto* button = dynamic_cast<NeoWPF::UI::Button*>(visual);
    if (button) {
        DumpButton(button, options, depth);
    }

    for (auto* child : visual->GetVisualChildren()) {
        DumpRecursive(child, options, depth + 1);
    }
}

void TextDumper::DumpTextBlock(NeoWPF::UI::TextBlock* textBlock, const TextDumpOptions& options, int depth)
{
    if (!textBlock) return;

    auto* fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(textBlock);
    LogInfo("%sTextBlock type=%s text='%s' fontSize=%d family='%ls'", GetIndent(depth), typeid(*textBlock).name(), SafeText(textBlock->GetText()).c_str(), textBlock->GetFontSize(), textBlock->GetFontFamily().c_str());
    if (options.includeLayout && fe) {
        LogInfo("%s  layout: desired=(%d,%d) final=(%d,%d) offset=(%d,%d)", GetIndent(depth), fe->GetDesiredWidth(), fe->GetDesiredHeight(), fe->GetFinalWidth(), fe->GetFinalHeight(), fe->GetLayoutOffsetX(), fe->GetLayoutOffsetY());
    }
    if (options.includePadding) {
        auto pad = textBlock->GetPadding();
        LogInfo("%s  padding=(%g,%g,%g,%g)", GetIndent(depth), pad.Left, pad.Top, pad.Right, pad.Bottom);
    }
    if (options.includeColors) {
        const auto foreground = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(textBlock->GetForeground());
        LogInfo("%s  foreground=%s", GetIndent(depth),
            foreground ? FormatArgb(foreground->GetArgb()).c_str() : "null");
    }
}

void TextDumper::DumpTextBox(NeoWPF::UI::TextBox* textBox, const TextDumpOptions& options, int depth)
{
    if (!textBox) return;

    auto* fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(textBox);
    LogInfo("%sTextBox type=%s text='%s' placeholder='%s' fontSize=%d family='%ls' readOnly=%d", GetIndent(depth), typeid(*textBox).name(), SafeText(textBox->GetText()).c_str(), SafeText(textBox->GetPlaceholderText()).c_str(), textBox->GetFontSize(), textBox->GetFontFamily().c_str(), textBox->IsReadOnly());
    if (options.includeLayout && fe) {
        LogInfo("%s  layout: desired=(%d,%d) final=(%d,%d) offset=(%d,%d)", GetIndent(depth), fe->GetDesiredWidth(), fe->GetDesiredHeight(), fe->GetFinalWidth(), fe->GetFinalHeight(), fe->GetLayoutOffsetX(), fe->GetLayoutOffsetY());
    }
    if (options.includePadding) {
        auto pad = textBox->GetPadding();
        LogInfo("%s  padding=(%g,%g,%g,%g)", GetIndent(depth), pad.Left, pad.Top, pad.Right, pad.Bottom);
    }
    if (options.includeColors) {
        const auto foreground = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(textBox->GetForeground());
        LogInfo("%s  foreground=%s bg=%s", GetIndent(depth),
            foreground ? FormatArgb(foreground->GetArgb()).c_str() : "null",
            FormatBrush(textBox->GetBackgroundBrush()).c_str());
    }
}

void TextDumper::DumpButton(NeoWPF::UI::Button* button, const TextDumpOptions& options, int depth)
{
    if (!button) return;

    auto* fe = dynamic_cast<NeoWPF::UI::FrameworkElement*>(button);
    LogInfo("%sButton type=%s text='%s' fontSize=%d", GetIndent(depth), typeid(*button).name(), SafeText(button->GetText()).c_str(), button->GetFontSize());
    if (options.includeLayout && fe) {
        LogInfo("%s  layout: desired=(%d,%d) final=(%d,%d) offset=(%d,%d)", GetIndent(depth), fe->GetDesiredWidth(), fe->GetDesiredHeight(), fe->GetFinalWidth(), fe->GetFinalHeight(), fe->GetLayoutOffsetX(), fe->GetLayoutOffsetY());
    }
    if (options.includePadding) {
        auto pad = button->GetPadding();
        LogInfo("%s  padding=(%g,%g,%g,%g)", GetIndent(depth), pad.Left, pad.Top, pad.Right, pad.Bottom);
    }
    if (options.includeColors) {
        LogInfo("%s  foreground=%s bg=%s border=%s", GetIndent(depth), FormatBrush(button->GetForeground()).c_str(), FormatBrush(button->GetBackgroundBrush()).c_str(), FormatBrush(button->GetBorderBrush()).c_str());
    }
}

const char* TextDumper::GetIndent(int depth)
{
    static thread_local std::string s_indentCache[32];
    if (depth < 0) depth = 0;
    if (depth >= 32) depth = 31;
    if (s_indentCache[depth].empty()) {
        s_indentCache[depth].assign(depth * 2, ' ');
    }
    return s_indentCache[depth].c_str();
}

}
