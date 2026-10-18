/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <memory>

#include <LuAudio/Audio/Playback/AudioMixer.h>

#include <NeoWPF/Media/Audio/IAudioMixer.h>

#include <Media/Backends/Audio/LuAudio/LASource.h>

namespace NeoWPF::Media::LA {

class LAMixer final : public IAudioMixer {
public:
    ~LAMixer() override;

    MediaResult OpenSource(const MediaSource& source, SourceId& id) override;
    MediaResult RemoveSource(SourceId id) override;

    MediaResult Play() override;
    MediaResult Pause() override;
    MediaResult Stop() override;
    MediaResult Seek(SourceId id, std::chrono::milliseconds position) override;
    MediaResult SetSourceVolume(SourceId id, float volume) override;
    MediaResult SetSourceLoop(SourceId id, bool enabled) override;

    MediaState GetState() const noexcept override;
    std::chrono::milliseconds GetSourcePosition(SourceId id) const noexcept override;
    bool IsSourceFinished(SourceId id) const noexcept override;
    std::size_t GetSourceCount() const noexcept override;
    const MediaResult& GetLastResult() const noexcept override;

private:
    MediaResult Store(MediaResult result);

    std::unique_ptr<LuAudio::Audio::IAudioBackend> backend_;
    std::unique_ptr<LuAudio::Audio::AudioMixer> mixer_;
    MediaState state_ = MediaState::Closed;
    MediaResult lastResult_ = MediaResult::Success();
    std::uint32_t sampleRate_ = 0;
};

}