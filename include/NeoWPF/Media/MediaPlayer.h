/*
MIT License — Copyright (c) 2026 NeoWPF
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

    MediaResult Open(const MediaSource& source);
    void Close() noexcept;

    MediaResult Play();
    MediaResult Pause();
    MediaResult Stop();
    MediaResult Seek(std::chrono::milliseconds position);
    MediaResult SetVolume(float volume);
    MediaResult SetLoop(bool enabled);

    MediaState GetState() const noexcept;
    std::chrono::milliseconds GetPosition() const noexcept;
    bool IsFinished() const noexcept;
    const MediaResult& GetLastResult() const noexcept;

private:
    std::unique_ptr<IAudioMixer> mixer_;
    std::optional<IAudioMixer::SourceId> sourceId_;
    MediaResult lastResult_ = MediaResult::Success();
};

}


