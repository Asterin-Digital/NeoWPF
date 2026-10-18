/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <functional>
#include <string>

namespace NeoWPF::UI {

/*
    @summary
    Provides access to the system clipboard for Unicode text.
*/
class Clipboard {
public:
    /*
        @summary
        Receives the result of an asynchronous clipboard text read.

        @param succeeded
        `true` when clipboard text was read successfully; otherwise `false`.

        @param text
        The clipboard text when the read succeeds, or an empty string otherwise.
    */
    using TextCallback = std::function<void(bool succeeded, std::wstring text)>;

    /*
        @summary
        Replaces the system clipboard contents with Unicode text.

        @param text
        The text to place on the clipboard.

        @returns
        `true` when the clipboard contents were replaced successfully; otherwise `false`.
    */
    bool SetText(const std::wstring& text);

    /*
        @summary
        Attempts to read Unicode text from the system clipboard asynchronously.

        @param callback
        Callback invoked later on NeoWPF's main thread with the read result.

        @note
        The callback is not invoked before this method returns.
    */
    void TryGetText(TextCallback callback);

    /*
        @summary
        Clears the system clipboard contents.

        @returns
        `true` when the clipboard was cleared successfully; otherwise `false`.
    */
    bool Clear();
};

}