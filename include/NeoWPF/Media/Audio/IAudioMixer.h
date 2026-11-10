/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <cstdint>

#include <NeoWPF/Etc/Result.h>
#include <NeoWPF/Media/Core/MediaError.h>
#include <NeoWPF/Media/Core/MediaSource.h>
#include <NeoWPF/Media/Core/MediaState.h>

namespace NeoWPF::Media {

class IAudioMixer {
public:
    using SourceId = std::uint32_t;

    virtual ~IAudioMixer() = default;

    virtual NeoWPF::Result<SourceId, MediaError> OpenSource(const MediaSource& source) = 0;
    virtual NeoWPF::Result<void, MediaError> RemoveSource(SourceId id) = 0;

    virtual NeoWPF::Result<void, MediaError> Play() = 0;
    virtual NeoWPF::Result<void, MediaError> Pause() = 0;
    virtual NeoWPF::Result<void, MediaError> Stop() = 0;
    virtual NeoWPF::Result<void, MediaError> Seek(SourceId id, std::chrono::milliseconds position) = 0;
    virtual NeoWPF::Result<void, MediaError> SetSourceVolume(SourceId id, float volume) = 0;
    virtual NeoWPF::Result<void, MediaError> SetSourceLoop(SourceId id, bool enabled) = 0;

    virtual MediaState GetState() const noexcept = 0;
    virtual std::chrono::milliseconds GetSourcePosition(SourceId id) const noexcept = 0;
    virtual bool IsSourceFinished(SourceId id) const noexcept = 0;
    virtual std::size_t GetSourceCount() const noexcept = 0;
    virtual const NeoWPF::Result<void, MediaError>& GetLastResult() const noexcept = 0;
};

}