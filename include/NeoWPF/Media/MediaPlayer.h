/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <memory>
#include <optional>

#include <NeoWPF/Media/Audio/IAudioMixer.h>

namespace NeoWPF::Media {

class MediaPlayer final {
public:
    MediaPlayer();
    ~MediaPlayer();

    NeoWPF::Result<void, MediaError> Open(const MediaSource& source);
    void Close() noexcept;

    NeoWPF::Result<void, MediaError> Play();
    NeoWPF::Result<void, MediaError> Pause();
    NeoWPF::Result<void, MediaError> Stop();
    NeoWPF::Result<void, MediaError> Seek(std::chrono::milliseconds position);
    NeoWPF::Result<void, MediaError> SetVolume(float volume);
    NeoWPF::Result<void, MediaError> SetLoop(bool enabled);

    MediaState GetState() const noexcept;
    std::chrono::milliseconds GetPosition() const noexcept;
    bool IsFinished() const noexcept;
    const NeoWPF::Result<void, MediaError>& GetLastResult() const noexcept;

private:
    std::unique_ptr<IAudioMixer> mixer_;
    std::optional<IAudioMixer::SourceId> sourceId_;
    NeoWPF::Result<void, MediaError> lastResult_ = NeoWPF::Success<void>{};
};

}


