/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <cstdint>

#include <NeoWPF/Media/Core/MediaResult.h>
#include <NeoWPF/Media/Core/MediaSource.h>
#include <NeoWPF/Media/Core/MediaState.h>

namespace NeoWPF::Media {

class IAudioMixer {
public:
    using SourceId = std::uint32_t;

    virtual ~IAudioMixer() = default;

    virtual MediaResult OpenSource(const MediaSource& source, SourceId& id) = 0;
    virtual MediaResult RemoveSource(SourceId id) = 0;

    virtual MediaResult Play() = 0;
    virtual MediaResult Pause() = 0;
    virtual MediaResult Stop() = 0;
    virtual MediaResult Seek(SourceId id, std::chrono::milliseconds position) = 0;
    virtual MediaResult SetSourceVolume(SourceId id, float volume) = 0;
    virtual MediaResult SetSourceLoop(SourceId id, bool enabled) = 0;

    virtual MediaState GetState() const noexcept = 0;
    virtual std::chrono::milliseconds GetSourcePosition(SourceId id) const noexcept = 0;
    virtual bool IsSourceFinished(SourceId id) const noexcept = 0;
    virtual std::size_t GetSourceCount() const noexcept = 0;
    virtual const MediaResult& GetLastResult() const noexcept = 0;
};

}