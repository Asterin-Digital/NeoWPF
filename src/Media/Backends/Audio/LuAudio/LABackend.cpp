#include <Media/Backends/Audio/LuAudio/LABackend.h>

#include <Media/Backends/Audio/LuAudio/LAMixer.h>

namespace NeoWPF::Media::LA {

std::string_view LABackend::GetName() const noexcept
{
    return "default-audio";
}

std::unique_ptr<IAudioMixer> LABackend::CreateAudioMixer(
    MediaResult& result)
{
    result = MediaResult::Success();
    return std::make_unique<LAMixer>();
}

}