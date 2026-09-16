#include <Media/Backends/Audio/LuAudio/LASource.h>

#include <NeoWPF/Platform/PlatformUtils.h>

#include <filesystem>
#include <unordered_map>

#include <LuAudio/Audio/Contracts/AudioBackendFactory.h>
#include <LuAudio/Audio/Sources/AudioFile.h>
#include <LuAudio/Audio/Sources/Mp3FileReader.h>
#include <LuAudio/Audio/Sources/OggFileReader.h>
#include <LuAudio/Audio/Sources/WavFileReader.h>

#include <Media/Backends/Audio/LuAudio/LAResult.h>

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

MediaResult OpenReader(
    const std::string& uri,
    SourceType sourceType,
    std::unique_ptr<LuAudio::Audio::IAudioReader>& reader)
{
    switch (sourceType) {
    case SourceType::Wav: {
        auto wavReader = std::make_unique<LuAudio::Audio::WavFileReader>();
        const auto result = wavReader->Open(uri);
        if (!result.Succeeded()) {
            return TranslateResult(result);
        }
        reader = std::move(wavReader);
        return MediaResult::Success();
    }

    case SourceType::Mp3: {
        auto decoder = LuAudio::Audio::CreateDecoder();
        if (!decoder) {
            return MediaResult::Failure(
                MediaResultCode::BackendUnavailable,
                "LuAudio did not provide an audio decoder");
        }

        auto mp3Reader = std::make_unique<LuAudio::Audio::Mp3FileReader>(
            std::move(decoder));
        const auto result = mp3Reader->Open(
            LuAudio::Audio::AudioFile(uri, LuAudio::Audio::AudioFileType::Mp3));
        if (!result.Succeeded()) {
            return TranslateResult(result);
        }
        reader = std::move(mp3Reader);
        return MediaResult::Success();
    }

    case SourceType::Ogg: {
        auto decoder = LuAudio::Audio::CreateDecoder();
        if (!decoder) {
            return MediaResult::Failure(
                MediaResultCode::BackendUnavailable,
                "LuAudio did not provide an audio decoder");
        }

        auto oggReader = std::make_unique<LuAudio::Audio::OggFileReader>(
            std::move(decoder));
        const auto result = oggReader->Open(
            LuAudio::Audio::AudioFile(uri, LuAudio::Audio::AudioFileType::Ogg));
        if (!result.Succeeded()) {
            return TranslateResult(result);
        }
        reader = std::move(oggReader);
        return MediaResult::Success();
    }

    default:
        return MediaResult::Failure(
            MediaResultCode::Unsupported,
            "Audio source type is not supported");
    }
}

}

MediaResult LASource::Open(const MediaSource& source)
{
    Close();
    if (source.IsEmpty()) {
        return MediaResult::Failure(
            MediaResultCode::InvalidArgument,
            "Audio source cannot be empty");
    }

    const auto& uri = source.GetUri();
    if (uri.find("://") != std::string::npos) {
        return MediaResult::Failure(
            MediaResultCode::Unsupported,
            "URI audio sources are not supported");
    }

    SourceType sourceType;
    const auto normalizedExtension = NeoWPF::Platform::ToLower(std::filesystem::path(uri).extension().string());
    if (!TryGetSourceType(normalizedExtension, sourceType)) {
        return MediaResult::Failure(
            MediaResultCode::Unsupported,
            "Audio file extension is not supported");
    }

    std::unique_ptr<LuAudio::Audio::IAudioReader> reader;
    const auto result = OpenReader(uri, sourceType, reader);
    if (!result.Succeeded()) {
        return result;
    }

    frameCount_ = reader->FrameCount();
    reader_ = std::move(reader);
    return MediaResult::Success();
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
