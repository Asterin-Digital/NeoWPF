/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <LuAudio/Audio/Contracts/Result.h>

#include <NeoWPF/Media/Core/MediaResult.h>

namespace NeoWPF::Media::LA {

MediaResult TranslateResult(const LuAudio::Audio::Result& result);

}
