#include <NeoWPF/Media/MediaPlayer.h>

#include <Media/Backends/Audio/LuAudio/LABackend.h>

namespace NeoWPF::Media {

MediaPlayer::MediaPlayer()
{
    LA::LABackend backend;
    MediaResult result;
    mixer_ = backend.CreateAudioMixer(result);
    if (!mixer_) {
        lastResult_ = std::move(result);
    }
}

MediaPlayer::~MediaPlayer()
{
    Close();
}

MediaResult MediaPlayer::Open(const MediaSource& source)
{
    if (!mixer_) {
        return lastResult_;
    }

    if (sourceId_.has_value()) {
        mixer_->RemoveSource(*sourceId_);
        sourceId_.reset();
    }

    IAudioMixer::SourceId sourceId = 0;
    lastResult_ = mixer_->OpenSource(source, sourceId);
    if (lastResult_.Succeeded()) {
        sourceId_ = sourceId;
    }
    return lastResult_;
}

void MediaPlayer::Close() noexcept
{
    if (mixer_ && sourceId_.has_value()) {
        mixer_->RemoveSource(*sourceId_);
        sourceId_.reset();
    }
}

MediaResult MediaPlayer::Play()
{
    lastResult_ = mixer_ ? mixer_->Play() : MediaResult::Failure(
        MediaResultCode::InvalidState,
        "Media player is not initialized");
    return lastResult_;
}

MediaResult MediaPlayer::Pause()
{
    lastResult_ = mixer_ ? mixer_->Pause() : MediaResult::Failure(
        MediaResultCode::InvalidState,
        "Media player is not initialized");
    return lastResult_;
}

MediaResult MediaPlayer::Stop()
{
    lastResult_ = mixer_ ? mixer_->Stop() : MediaResult::Failure(
        MediaResultCode::InvalidState,
        "Media player is not initialized");
    return lastResult_;
}

MediaResult MediaPlayer::Seek(std::chrono::milliseconds position)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->Seek(*sourceId_, position)
        : MediaResult::Failure(
            MediaResultCode::InvalidState,
            "Media player has no open source");
    return lastResult_;
}

MediaResult MediaPlayer::SetVolume(float volume)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->SetSourceVolume(*sourceId_, volume)
        : MediaResult::Failure(
            MediaResultCode::InvalidState,
            "Media player has no open source");
    return lastResult_;
}

MediaResult MediaPlayer::SetLoop(bool enabled)
{
    lastResult_ = mixer_ && sourceId_.has_value()
        ? mixer_->SetSourceLoop(*sourceId_, enabled)
        : MediaResult::Failure(
            MediaResultCode::InvalidState,
            "Media player has no open source");
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

const MediaResult& MediaPlayer::GetLastResult() const noexcept
{
    return mixer_ ? mixer_->GetLastResult() : lastResult_;
}

}
