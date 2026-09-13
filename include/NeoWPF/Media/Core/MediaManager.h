/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <NeoWPF/Media/Audio/IAudioMixer.h>
#include <NeoWPF/Media/Backends/IMediaBackend.h>
#include <NeoWPF/Media/Core/MediaResult.h>

namespace NeoWPF::Media {

class MediaManager final {
public:
    MediaResult RegisterBackend(std::shared_ptr<IMediaBackend> backend);

    MediaResult UnregisterBackend(const std::string& name);

    std::unique_ptr<IAudioMixer> CreateAudioMixer(
        const std::string& backendName,
        MediaResult& result) const;

    bool HasBackend(const std::string& name) const noexcept;

private:
    std::unordered_map<std::string, std::shared_ptr<IMediaBackend>> backends_;
};

}
