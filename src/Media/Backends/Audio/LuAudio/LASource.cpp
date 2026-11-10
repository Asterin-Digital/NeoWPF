#include <NeoWPF/Media/Backends/Audio/LuAudio/LASource.h>

#include <NeoWPF/Platform/PlatformUtils.h>

#include <filesystem>
#include <unordered_map>

#include <LuAudio/Audio/Contracts/AudioBackendFactory.h>
#include <LuAudio/Audio/Sources/AudioFile.h>
#include <LuAudio/Audio/Sources/Mp3FileReader.h>
#include <LuAudio/Audio/Sources/OggFileReader.h>
#include <LuAudio/Audio/Sources/WavFileReader.h>

#include <NeoWPF/Media/Backends/Audio/LuAudio/LAResult.h>

namespace NeoWPF::Media::LA {

namespace {

enum class SourceType {
    Wav,
    Mp3,
    Ogg,
};

bool TryGetSourceType(const std::string& extension, SourceType& type)
{
    static const std::unordered_map<std::string, SourceType> sourceTypes{
        {".wav", SourceType::Wav},
        {".mp3", SourceType::Mp3},
        {".ogg", SourceType::Ogg},
    };

    const auto iterator = sourceTypes.find(extension);
    if (iterator == sourceTypes.end()) {
        return false;
    }

    type = iterator->second;
    return true;
}

NeoWPF::Result<std::unique_ptr<LuAudio::Audio::IAudioReader>, MediaError> OpenReader(
    const std::string& uri,
    SourceType sourceType)
{
    switch (sourceType) {
    case SourceType::Wav: {
        auto wavReader = std::make_unique<LuAudio::Audio::WavFileReader>();
        const auto translated = TranslateResult(wavReader->Open(uri));
        if (translated.Fail()) {
            return NeoWPF::Fail<MediaError>{translated.Error()};
        }
        return NeoWPF::Success<std::unique_ptr<LuAudio::Audio::IAudioReader>>{
            std::move(wavReader)};
    }

    case SourceType::Mp3: {
        auto decoder = LuAudio::Audio::CreateDecoder();
        if (!decoder) {
            return NeoWPF::Fail<MediaError>{MediaError{
                MediaErrorCode::BackendUnavailable,
                "LuAudio did not provide an audio decoder"}};
        }

        auto mp3Reader = std::make_unique<LuAudio::Audio::Mp3FileReader>(
            std::move(decoder));
        const auto translated = TranslateResult(mp3Reader->Open(
            LuAudio::Audio::AudioFile(uri, LuAudio::Audio::AudioFileType::Mp3)));
        if (translated.Fail()) {
            return NeoWPF::Fail<MediaError>{translated.Error()};
        }
        return NeoWPF::Success<std::unique_ptr<LuAudio::Audio::IAudioReader>>{
            std::move(mp3Reader)};
    }

    case SourceType::Ogg: {
        auto decoder = LuAudio::Audio::CreateDecoder();
        if (!decoder) {
            return NeoWPF::Fail<MediaError>{MediaError{
                MediaErrorCode::BackendUnavailable,
                "LuAudio did not provide an audio decoder"}};
        }

        auto oggReader = std::make_unique<LuAudio::Audio::OggFileReader>(
            std::move(decoder));
        const auto translated = TranslateResult(oggReader->Open(
            LuAudio::Audio::AudioFile(uri, LuAudio::Audio::AudioFileType::Ogg)));
        if (translated.Fail()) {
            return NeoWPF::Fail<MediaError>{translated.Error()};
        }
        return NeoWPF::Success<std::unique_ptr<LuAudio::Audio::IAudioReader>>{
            std::move(oggReader)};
    }

    default:
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::Unsupported,
            "Audio source type is not supported"}};
    }
}

}

NeoWPF::Result<void, MediaError> LASource::Open(const MediaSource& source)
{
    Close();
    if (source.IsEmpty()) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::InvalidArgument,
            "Audio source cannot be empty"}};
    }

    const auto& uri = source.GetUri();
    if (uri.find("://") != std::string::npos) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::Unsupported,
            "URI audio sources are not supported"}};
    }

    SourceType sourceType;
    const auto normalizedExtension = NeoWPF::Platform::ToLower(std::filesystem::path(uri).extension().string());
    if (!TryGetSourceType(normalizedExtension, sourceType)) {
        return NeoWPF::Fail<MediaError>{MediaError{
            MediaErrorCode::Unsupported,
            "Audio file extension is not supported"}};
    }

    auto result = OpenReader(uri, sourceType);
    if (result.Fail()) {
        return NeoWPF::Fail<MediaError>{result.Error()};
    }

    auto reader = std::move(result).Value();
    frameCount_ = reader->FrameCount();
    reader_ = std::move(reader);
    return NeoWPF::Success<void>{};
}

void LASource::Close() noexcept
{
    reader_.reset();
    frameCount_ = 0;
}

std::unique_ptr<LuAudio::Audio::IAudioReader> LASource::TakeReader() noexcept
{
    return std::move(reader_);
}

std::uint64_t LASource::GetFrameCount() const noexcept
{
    return frameCount_;
}

}
