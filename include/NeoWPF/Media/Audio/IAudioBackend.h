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