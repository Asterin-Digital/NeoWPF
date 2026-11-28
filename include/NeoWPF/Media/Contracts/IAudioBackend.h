/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <memory>

#include <NeoWPF/Media/Audio/IAudioMixer.h>
#include <NeoWPF/Media/Backends/IMediaBackend.h>
#include <NeoWPF/Feedback/Result.h>
#include <NeoWPF/Media/Core/MediaError.h>

namespace NeoWPF::Media {

class IAudioBackend : public IMediaBackend {
public:
    ~IAudioBackend() override = default;

    virtual NeoWPF::Result<std::unique_ptr<IAudioMixer>, MediaError> CreateAudioMixer() = 0;
};

}