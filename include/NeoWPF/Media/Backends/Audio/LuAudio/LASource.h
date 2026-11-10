/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <memory>

#include <LuAudio/Audio/Sources/IAudioReader.h>

#include <NeoWPF/Etc/Result.h>
#include <NeoWPF/Media/Core/MediaError.h>
#include <NeoWPF/Media/Core/MediaSource.h>

namespace NeoWPF::Media::LA {

class LASource final {
public:
    NeoWPF::Result<void, MediaError> Open(const MediaSource& source);
    void Close() noexcept;

    std::unique_ptr<LuAudio::Audio::IAudioReader> TakeReader() noexcept;
    std::uint64_t GetFrameCount() const noexcept;

private:
    std::unique_ptr<LuAudio::Audio::IAudioReader> reader_;
    std::uint64_t frameCount_ = 0;
};

}
