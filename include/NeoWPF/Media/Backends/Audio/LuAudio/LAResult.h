/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <LuAudio/Audio/Contracts/Result.h>

#include <NeoWPF/Etc/Result.h>
#include <NeoWPF/Media/Core/MediaError.h>

namespace NeoWPF::Media::LA {

NeoWPF::Result<void, MediaError> TranslateResult(const LuAudio::Audio::Result& result);

}
