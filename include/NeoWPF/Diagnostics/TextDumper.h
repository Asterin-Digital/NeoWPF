/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Button/Button.h>

namespace NeoWPF::Diagnostics {

struct TextDumpOptions {
    bool includeLayout = true;
    bool includeColors = true;
    bool includePadding = true;
};

class TextDumper {
public:
    static void Dump(NeoWPF::UI::Visual* root, const TextDumpOptions& options = TextDumpOptions{});

private:
    TextDumper() = delete;

    static void DumpRecursive(NeoWPF::UI::Visual* visual, const TextDumpOptions& options, int depth);
    static void DumpTextBlock(NeoWPF::UI::TextBlock* textBlock, const TextDumpOptions& options, int depth);
    static void DumpTextBox(NeoWPF::UI::TextBox* textBox, const TextDumpOptions& options, int depth);
    static void DumpButton(NeoWPF::UI::Button* button, const TextDumpOptions& options, int depth);
    static const char* GetIndent(int depth);
};

}
