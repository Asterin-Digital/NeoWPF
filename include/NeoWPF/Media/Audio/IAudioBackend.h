/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>

#include <NeoWPF/Media/Audio/IAudioMixer.h>
#include <NeoWPF/Media/Backends/IMediaBackend.h>
#include <NeoWPF/Media/Core/MediaResult.h>

namespace NeoWPF::Media {

class IAudioBackend : public IMediaBackend {
public:
    ~IAudioBackend() override = default;

    virtual std::unique_ptr<IAudioMixer> CreateAudioMixer(
        MediaResult& result) = 0;
};

}