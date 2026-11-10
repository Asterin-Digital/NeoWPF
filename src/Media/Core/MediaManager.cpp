#include <NeoWPF/Media/Core/MediaManager.h>

#include <NeoWPF/Media/Audio/IAudioBackend.h>

namespace NeoWPF::Media {

NeoWPF::Result<void, MediaError> MediaManager::RegisterBackend(std::shared_ptr<IMediaBackend> backend)
{
    if (!backend) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Media backend cannot be null"}};
    }

    const std::string name(backend->GetName());
    if (name.empty()) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Media backend name cannot be empty"}};
    }

    if (backends_.contains(name)) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Media backend is already registered"}};
    }

    backends_.emplace(name, std::move(backend));
    return NeoWPF::Success<void>{};
}

NeoWPF::Result<void, MediaError> MediaManager::UnregisterBackend(const std::string& name)
{
    if (name.empty()) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Media backend name cannot be empty"}};
    }

    if (backends_.erase(name) == 0) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Media backend is not registered"}};
    }

    return NeoWPF::Success<void>{};
}

std::shared_ptr<IMediaBackend> MediaManager::GetBackend(const std::string& name) const
{
    if (name.empty()) {
        return nullptr;
    }

    const auto iterator = backends_.find(name);
    if (iterator == backends_.end()) {
        return nullptr;
    }

    return iterator->second;
}

NeoWPF::Result<std::unique_ptr<IAudioMixer>, MediaError> MediaManager::CreateAudioMixer(
    const std::string& backendName) const
{
    const auto iterator = backends_.find(backendName);
    if (iterator == backends_.end()) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::BackendUnavailable,
            "Requested media backend is not registered"}};
    }

    auto* audioBackend = dynamic_cast<IAudioBackend*>(iterator->second.get());
    if (audioBackend == nullptr) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::Unsupported,
            "Requested media backend does not provide audio mixing"}};
    }

    auto result = audioBackend->CreateAudioMixer();
    if (result.Success() && !result.Value()) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::BackendUnavailable,
            "Media backend did not create an audio mixer"}};
    }
    return result;
}

bool MediaManager::HasBackend(const std::string& name) const noexcept
{
    return backends_.contains(name);
}

}
