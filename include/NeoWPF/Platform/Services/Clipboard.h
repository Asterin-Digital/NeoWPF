/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <functional>
#include <string>

#include <NeoWPF/Contracts/IGeneral.h>

namespace NeoWPF::Platform::Services {

/*
    @summary
    Provides access to the system clipboard for Unicode text.
*/
class Clipboard {
public:
    /*
        @summary
        Receives the result of an asynchronous clipboard text read.

        @param result
        The clipboard text, an empty result when no text is available, or a failure reason.
    */
    using TextCallback = NeoWPF::Contracts::IClipboard::TextCallback;

    /*
        @summary
        Replaces the system clipboard contents with Unicode text.

        @param text
        The text to place on the clipboard.

        @returns
        Success when the clipboard contents were replaced, or the failure reason.
    */
    NeoWPF::Result<void> SetText(const std::wstring& text);

    /*
        @summary
        Attempts to read Unicode text from the system clipboard asynchronously.

        @param callback
        Callback invoked later on NeoWPF's main thread with the read result.
    */
    void TryGetText(TextCallback callback);

    /*
        @summary
        Clears the system clipboard contents.

        @returns
        Success when the clipboard was cleared, or the failure reason.
    */
    NeoWPF::Result<void> Clear();
};

}