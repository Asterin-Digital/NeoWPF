#include <NeoWPF/Media/Core/MediaManager.h>

#include <NeoWPF/Media/Audio/IAudioBackend.h>

namespace NeoWPF::Media {

MediaResult MediaManager::RegisterBackend(std::shared_ptr<IMediaBackend> backend)
{
    if (!backend) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Media backend cannot be null");
    }

    const std::string name(backend->GetName());
    if (name.empty()) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Media backend name cannot be empty");
    }

    if (backends_.contains(name)) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Media backend is already registered");
    }

    backends_.emplace(name, std::move(backend));
    return MediaResult::Success();
}

MediaResult MediaManager::UnregisterBackend(const std::string& name)
{
    if (name.empty()) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Media backend name cannot be empty");
    }

    if (backends_.erase(name) == 0) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Media backend is not registered");
    }

    return MediaResult::Success();
}

std::unique_ptr<IAudioMixer> MediaManager::CreateAudioMixer(
    const std::string& backendName,
    MediaResult& result) const
{
    const auto iterator = backends_.find(backendName);
    if (iterator == backends_.end()) {
        result = MediaResult::Failure(
            MediaResultCode::BackendUnavailable,
            "Requested media backend is not registered");
        return nullptr;
    }

    auto* audioBackend = dynamic_cast<IAudioBackend*>(iterator->second.get());
    if (audioBackend == nullptr) {
        result = MediaResult::Failure(
            MediaResultCode::Unsupported,
            "Requested media backend does not provide audio mixing");
        return nullptr;
    }

    auto mixer = audioBackend->CreateAudioMixer(result);
    if (!mixer && result.Succeeded()) {
        result = MediaResult::Failure(
            MediaResultCode::BackendUnavailable,
            "Media backend did not create an audio mixer");
    }
    return mixer;
}

bool MediaManager::HasBackend(const std::string& name) const noexcept
{
    return backends_.contains(name);
}

}
