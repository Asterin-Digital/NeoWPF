/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string_view>

#include <NeoWPF/Media/Audio/IAudioBackend.h>

namespace NeoWPF::Media::LA {

class LABackend final : public IAudioBackend {
public:
    std::string_view GetName() const noexcept override;

    std::unique_ptr<IAudioMixer> CreateAudioMixer(
        MediaResult& result) override;
};

}