#include <Media/Backends/Audio/LuAudio/LABackend.h>

#include <Media/Backends/Audio/LuAudio/LAMixer.h>

namespace NeoWPF::Media::LA {

LABackend::LABackend()
    : nativeBackend_(LuAudio::Audio::CreateBackend())
{
}

LABackend::~LABackend() = default;

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

LuAudio::Audio::Result LABackend::Open(const LuAudio::Audio::AudioStreamConfig& requestedConfig)
{
    if (!nativeBackend_) {
        return LuAudio::Audio::Result::Failure(
            LuAudio::Audio::ResultCode::BackendUnavailable,
            "LuAudio backend was not created");
    }
    return nativeBackend_->Open(requestedConfig);
}

LuAudio::Audio::Result LABackend::Start()
{
    if (!nativeBackend_) {
        return LuAudio::Audio::Result::Failure(
            LuAudio::Audio::ResultCode::BackendUnavailable,
            "LuAudio backend was not created");
    }
    return nativeBackend_->Start();
}

LuAudio::Audio::Result LABackend::Recover()
{
    if (!nativeBackend_) {
        return LuAudio::Audio::Result::Failure(
            LuAudio::Audio::ResultCode::BackendUnavailable,
            "LuAudio backend was not created");
    }
    return nativeBackend_->Recover();
}

LuAudio::Audio::Result LABackend::Stop()
{
    if (!nativeBackend_) {
        return LuAudio::Audio::Result::Failure(
            LuAudio::Audio::ResultCode::BackendUnavailable,
            "LuAudio backend was not created");
    }
    return nativeBackend_->Stop();
}

void LABackend::Close() noexcept
{
    if (nativeBackend_) {
        nativeBackend_->Close();
    }
}

void LABackend::SetCallback(LuAudio::Audio::AudioCallback callback)
{
    if (nativeBackend_) {
        nativeBackend_->SetCallback(std::move(callback));
    }
}

const LuAudio::Audio::AudioStreamConfig& LABackend::ActualConfig() const noexcept
{
    static const LuAudio::Audio::AudioStreamConfig emptyConfig{};
    return nativeBackend_ ? nativeBackend_->ActualConfig() : emptyConfig;
}

}