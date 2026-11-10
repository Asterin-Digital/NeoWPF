#include <NeoWPF/Media/Backends/Audio/LuAudio/LAMixer.h>

#include <cmath>
#include <limits>

#include <LuAudio/Audio/Contracts/AudioBackendFactory.h>

#include <NeoWPF/Media/Backends/Audio/LuAudio/LAResult.h>

namespace NeoWPF::Media::LA {

namespace {

std::uint64_t ToFrames(std::chrono::milliseconds position, std::uint32_t sampleRate)
{
    if (position.count() <= 0 || sampleRate == 0) {
        return 0;
    }
    const long double frames = static_cast<long double>(position.count()) *
        static_cast<long double>(sampleRate) / 1000.0L;
    if (frames >= static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
        return std::numeric_limits<std::uint64_t>::max();
    }
    return static_cast<std::uint64_t>(frames);
}

std::chrono::milliseconds ToMilliseconds(std::uint64_t frames, std::uint32_t sampleRate)
{
    if (sampleRate == 0) {
        return {};
    }
    const long double milliseconds = static_cast<long double>(frames) * 1000.0L /
        static_cast<long double>(sampleRate);
    if (milliseconds >= static_cast<long double>(
            std::numeric_limits<std::chrono::milliseconds::rep>::max())) {
        return std::chrono::milliseconds::max();
    }
    return std::chrono::milliseconds(
        static_cast<std::chrono::milliseconds::rep>(milliseconds));
}

}

LAMixer::~LAMixer()
{
    if (mixer_) {
        mixer_->Close();
    }
}

NeoWPF::Result<IAudioMixer::SourceId, MediaError> LAMixer::OpenSource(const MediaSource& source)
{
    LASource audioSource;
    auto result = audioSource.Open(source);
    if (result.Fail()) {
        lastResult_ = NeoWPF::Fail<MediaError>{result.Error()};
        return NeoWPF::Fail<MediaError>{result.Error()};
    }

    auto reader = audioSource.TakeReader();
    if (!reader) {
        lastResult_ = NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio source did not produce a reader"}};
        return NeoWPF::Fail<MediaError>{lastResult_.Error()};
    }

    if (!mixer_) {
        backend_ = LuAudio::Audio::CreateBackend();
        if (!backend_) {
            lastResult_ = NeoWPF::Fail<MediaError>{MediaError{
                MediaErrorCode::BackendUnavailable,
                "LuAudio did not provide an audio backend"}};
            return NeoWPF::Fail<MediaError>{lastResult_.Error()};
        }

        LuAudio::Audio::AudioStreamConfig config;
        config.format = reader->Format();
        sampleRate_ = config.format.sampleRate;
        mixer_ = std::make_unique<LuAudio::Audio::AudioMixer>(*backend_, 64);
        result = TranslateResult(mixer_->Open(config));
        if (result.Fail()) {
            mixer_.reset();
            backend_.reset();
            lastResult_ = NeoWPF::Fail<MediaError>{result.Error()};
            return NeoWPF::Fail<MediaError>{result.Error()};
        }
        state_ = MediaState::Stopped;
    }

    SourceId id = 0;
    result = TranslateResult(mixer_->AddSource(std::move(reader), id));
    if (result.Fail()) {
        lastResult_ = NeoWPF::Fail<MediaError>{result.Error()};
        return NeoWPF::Fail<MediaError>{result.Error()};
    }
    lastResult_ = NeoWPF::Success<void>{};
    return NeoWPF::Success<SourceId>{id};
}

NeoWPF::Result<void, MediaError> LAMixer::RemoveSource(SourceId id)
{
    if (!mixer_) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio mixer is not open"}});
    }
    return Store(TranslateResult(mixer_->RemoveSource(id)));
}

NeoWPF::Result<void, MediaError> LAMixer::Play()
{
    if (!mixer_) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio mixer is not open"}});
    }
    const auto result = TranslateResult(mixer_->Start());
    if (result.Success()) {
        state_ = MediaState::Playing;
    }
    return Store(result);
}

NeoWPF::Result<void, MediaError> LAMixer::Pause()
{
    if (!mixer_) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio mixer is not open"}});
    }
    const auto result = TranslateResult(mixer_->Stop());
    if (result.Success()) {
        state_ = MediaState::Paused;
    }
    return Store(result);
}

NeoWPF::Result<void, MediaError> LAMixer::Stop()
{
    if (!mixer_) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio mixer is not open"}});
    }
    const auto result = TranslateResult(mixer_->Stop());
    if (result.Success()) {
        state_ = MediaState::Stopped;
    }
    return Store(result);
}

NeoWPF::Result<void, MediaError> LAMixer::Seek(SourceId id, std::chrono::milliseconds position)
{
    if (!mixer_) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidState,
            "Audio mixer is not open"}});
    }
    if (position.count() < 0) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Audio seek position cannot be negative"}});
    }
    return Store(TranslateResult(mixer_->SeekSource(
        id, ToFrames(position, sampleRate_))));
}

NeoWPF::Result<void, MediaError> LAMixer::SetSourceVolume(SourceId id, float volume)
{
    if (!std::isfinite(volume) || volume < 0.0F) {
        return Store(NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Audio source volume cannot be negative or non-finite"}});
    }
    return Store(TranslateResult(mixer_->SetSourceGain(id, volume)));
}

NeoWPF::Result<void, MediaError> LAMixer::SetSourceLoop(SourceId, bool)
{
    return Store(NeoWPF::Fail<MediaError>{MediaError{
        MediaErrorCode::Unsupported,
        "Audio source looping is not implemented yet"}});
}

MediaState LAMixer::GetState() const noexcept
{
    return state_;
}

std::chrono::milliseconds LAMixer::GetSourcePosition(SourceId id) const noexcept
{
    return mixer_ ? ToMilliseconds(mixer_->SourcePosition(id), sampleRate_) :
        std::chrono::milliseconds{};
}

bool LAMixer::IsSourceFinished(SourceId id) const noexcept
{
    return mixer_ && mixer_->IsSourceFinished(id);
}

std::size_t LAMixer::GetSourceCount() const noexcept
{
    return mixer_ ? mixer_->ActiveSourceCount() : 0;
}

const NeoWPF::Result<void, MediaError>& LAMixer::GetLastResult() const noexcept
{
    return lastResult_;
}

NeoWPF::Result<void, MediaError> LAMixer::Store(
    NeoWPF::Result<void, MediaError> result)
{
    lastResult_ = std::move(result);
    return lastResult_;
}

}