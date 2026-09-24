/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string_view>

#include <LuAudio/Audio/Contracts/AudioBackendFactory.h>
#include <LuAudio/Audio/Contracts/IAudioBackend.h>

#include <NeoWPF/Media/Audio/IAudioBackend.h>

namespace NeoWPF::Media::LA {

class LABackend final : public IAudioBackend, public LuAudio::Audio::IAudioBackend {
public:
    LABackend();
    ~LABackend() override;

    std::string_view GetName() const noexcept override;

    std::unique_ptr<IAudioMixer> CreateAudioMixer(
        MediaResult& result) override;

    LuAudio::Audio::Result Open(const LuAudio::Audio::AudioStreamConfig& requestedConfig) override;
    LuAudio::Audio::Result Start() override;
    LuAudio::Audio::Result Recover() override;
    LuAudio::Audio::Result Stop() override;
    void Close() noexcept override;
    void SetCallback(LuAudio::Audio::AudioCallback callback) override;
    const LuAudio::Audio::AudioStreamConfig& ActualConfig() const noexcept override;

private:
    std::unique_ptr<LuAudio::Audio::IAudioBackend> nativeBackend_;
};

}