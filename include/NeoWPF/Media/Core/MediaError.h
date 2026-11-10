/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>

namespace NeoWPF::Media {

enum class MediaErrorCode {
    InvalidArgument,
    InvalidState,
    Unsupported,
    BackendUnavailable,
    ProcessingFailed,
};

struct MediaError final {
    MediaErrorCode Code;
    std::string Message;
};

}