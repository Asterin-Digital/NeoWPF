/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <memory>

#include <LuAudio/Audio/Playback/AudioMixer.h>

#include <NeoWPF/Media/Audio/IAudioMixer.h>

#include <NeoWPF/Media/Backends/Audio/LuAudio/LASource.h>

namespace NeoWPF::Media::LA {

class LAMixer final : public IAudioMixer {
public:
    ~LAMixer() override;

    NeoWPF::Result<SourceId, MediaError> OpenSource(const MediaSource& source) override;
    NeoWPF::Result<void, MediaError> RemoveSource(SourceId id) override;

    NeoWPF::Result<void, MediaError> Play() override;
    NeoWPF::Result<void, MediaError> Pause() override;
    NeoWPF::Result<void, MediaError> Stop() override;
    NeoWPF::Result<void, MediaError> Seek(SourceId id, std::chrono::milliseconds position) override;
    NeoWPF::Result<void, MediaError> SetSourceVolume(SourceId id, float volume) override;
    NeoWPF::Result<void, MediaError> SetSourceLoop(SourceId id, bool enabled) override;

    MediaState GetState() const noexcept override;
    std::chrono::milliseconds GetSourcePosition(SourceId id) const noexcept override;
    bool IsSourceFinished(SourceId id) const noexcept override;
    std::size_t GetSourceCount() const noexcept override;
    const NeoWPF::Result<void, MediaError>& GetLastResult() const noexcept override;

private:
    NeoWPF::Result<void, MediaError> Store(NeoWPF::Result<void, MediaError> result);

    std::unique_ptr<LuAudio::Audio::IAudioBackend> backend_;
    std::unique_ptr<LuAudio::Audio::AudioMixer> mixer_;
    MediaState state_ = MediaState::Closed;
    NeoWPF::Result<void, MediaError> lastResult_ = NeoWPF::Success<void>{};
    std::uint32_t sampleRate_ = 0;
};

}