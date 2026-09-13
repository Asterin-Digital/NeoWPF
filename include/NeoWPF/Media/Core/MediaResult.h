/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <utility>

namespace NeoWPF::Media {

enum class MediaResultCode {
    Success,
    InvalidArgument,
    InvalidState,
    Unsupported,
    BackendUnavailable,
    ProcessingFailed,
};

class MediaResult final {
public:
    MediaResult() noexcept
        : code_(MediaResultCode::Success), message_{}
    {
    }

    static MediaResult Success() noexcept
    {
        return MediaResult(MediaResultCode::Success, {});
    }

    static MediaResult Failure(MediaResultCode code, std::string message)
    {
        return MediaResult(code, std::move(message));
    }

    bool Succeeded() const noexcept
    {
        return code_ == MediaResultCode::Success;
    }

    MediaResultCode Code() const noexcept
    {
        return code_;
    }

    const std::string& Message() const noexcept
    {
        return message_;
    }

private:
    MediaResult(MediaResultCode code, std::string message)
        : code_(code), message_(std::move(message))
    {
    }

    MediaResultCode code_;
    std::string message_;
};

}
