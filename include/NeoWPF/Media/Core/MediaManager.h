/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <NeoWPF/Media/Audio/IAudioMixer.h>
#include <NeoWPF/Media/Backends/IMediaBackend.h>
#include <NeoWPF/Etc/Result.h>
#include <NeoWPF/Media/Core/MediaError.h>

namespace NeoWPF::Media {

class MediaManager final {
public:
    NeoWPF::Result<void, MediaError> RegisterBackend(std::shared_ptr<IMediaBackend> backend);

    NeoWPF::Result<void, MediaError> UnregisterBackend(const std::string& name);

    std::shared_ptr<IMediaBackend> GetBackend(const std::string& name) const;

    template<typename T>
    T* TryGetNativeBackend(const std::string& name) noexcept
    {
        const auto backend = GetBackend(name);
        return backend ? backend->TryGetNativeBackend<T>() : nullptr;
    }

    template<typename T>
    const T* TryGetNativeBackend(const std::string& name) const noexcept
    {
        const auto backend = GetBackend(name);
        return backend ? backend->TryGetNativeBackend<T>() : nullptr;
    }

    NeoWPF::Result<std::unique_ptr<IAudioMixer>, MediaError> CreateAudioMixer(
        const std::string& backendName) const;

    bool HasBackend(const std::string& name) const noexcept;

private:
    std::unordered_map<std::string, std::shared_ptr<IMediaBackend>> backends_;
};

}
