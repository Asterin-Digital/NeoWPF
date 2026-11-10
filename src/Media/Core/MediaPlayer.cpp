#include <NeoWPF/Media/MediaPlayer.h>

#include <NeoWPF/Media/Backends/Audio/LuAudio/LABackend.h>

namespace NeoWPF::Media {

MediaPlayer::MediaPlayer()
{
    LA::LABackend backend;
    auto result = backend.CreateAudioMixer();
    if (result.Fail()) {
        lastResult_ = NeoWPF::Fail<MediaError>{result.Error()};
    } else {
        mixer_ = std::move(result).Value();
    }
}

MediaPlayer::~MediaPlayer()
{
    Close();
}

NeoWPF::Result<void, MediaError> MediaPlayer::Open(const MediaSource& source)
{
    if (!mixer_) {
        return lastResult_;
    }

    if (sourceId_.has_value()) {
        const auto removeResult = mixer_->RemoveSource(*sourceId_);
        if (removeResult.Fail()) {
            lastResult_ = NeoWPF::Fail<MediaError>{removeResult.Error()};
            return lastResult_;
        }
        sourceId_.reset();
    }

    auto result = mixer_->OpenSource(source);
    if (result.Fail()) {
        lastResult_ = NeoWPF::Fail<MediaError>{result.Error()};
        return lastResult_;
    }
    sourceId_ = std::move(result).Value();
    lastResult_ = NeoWPF::Success<void>{};
    return lastResult_;
}

void MediaPlayer::Close() noexcept
{
    if (mixer_ && sourceId_.has_value()) {
        mixer_->RemoveSource(*sourceId_);
        sourceId_.reset();
    }
}

NeoWPF::Result<void, MediaError> MediaPlayer::Play()
{
    lastResult_ = mixer_ ? mixer_->Play() : NeoWPF::Result<void, MediaError>(
        NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player is not initialized"}});
    return lastResult_;
}

NeoWPF::Result<void, MediaError> MediaPlayer::Pause()
{
    lastResult_ = mixer_ ? mixer_->Pause() : NeoWPF::Result<void, MediaError>(
        NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player is not initialized"}});
    return lastResult_;
}

NeoWPF::Result<void, MediaError> MediaPlayer::Stop()
{
    lastResult_ = mixer_ ? mixer_->Stop() : NeoWPF::Result<void, MediaError>(
        NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player is not initialized"}});
    return lastResult_;
}

NeoWPF::Result<void, MediaError> MediaPlayer::Seek(std::chrono::milliseconds position)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->Seek(*sourceId_, position)
        : NeoWPF::Result<void, MediaError>(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player has no open source"}});
    return lastResult_;
}

NeoWPF::Result<void, MediaError> MediaPlayer::SetVolume(float volume)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->SetSourceVolume(*sourceId_, volume)
        : NeoWPF::Result<void, MediaError>(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player has no open source"}});
    return lastResult_;
}

NeoWPF::Result<void, MediaError> MediaPlayer::SetLoop(bool enabled)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->SetSourceLoop(*sourceId_, enabled)
        : NeoWPF::Result<void, MediaError>(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Media player has no open source"}});
    return lastResult_;
}

MediaState MediaPlayer::GetState() const noexcept
{
    return mixer_ ? mixer_->GetState() : MediaState::Failed;
}

std::chrono::milliseconds MediaPlayer::GetPosition() const noexcept
{
    return mixer_ && sourceId_.has_value()
        ? mixer_->GetSourcePosition(*sourceId_)
        : std::chrono::milliseconds{};
}

bool MediaPlayer::IsFinished() const noexcept
{
    return mixer_ && sourceId_.has_value() && mixer_->IsSourceFinished(*sourceId_);
}

const NeoWPF::Result<void, MediaError>& MediaPlayer::GetLastResult() const noexcept
{
    return mixer_ ? mixer_->GetLastResult() : lastResult_;
}

}
