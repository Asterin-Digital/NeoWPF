#pragma once

#include <LuAudio/Audio/Contracts/Result.h>

#include <NeoWPF/Media/Core/MediaResult.h>

namespace NeoWPF::Media::LA {

MediaResult TranslateResult(const LuAudio::Audio::Result& result);

}
