#include <NeoWPF/Media/Backends/Audio/LuAudio/LAResult.h>

namespace NeoWPF::Media::LA {

NeoWPF::Result<void, MediaError> TranslateResult(const LuAudio::Audio::Result& result)
{
    if (result.Succeeded()) {
        return NeoWPF::Success<void>{};
    }

    MediaErrorCode code = MediaErrorCode::ProcessingFailed;
    switch (result.Code()) {
    case LuAudio::Audio::ResultCode::InvalidArgument:
        code = MediaErrorCode::InvalidArgument;
        break;
    case LuAudio::Audio::ResultCode::InvalidState:
        code = MediaErrorCode::InvalidState;
        break;
    case LuAudio::Audio::ResultCode::BackendUnavailable:
        code = MediaErrorCode::BackendUnavailable;
        break;
    case LuAudio::Audio::ResultCode::ProcessingFailed:
        code = MediaErrorCode::ProcessingFailed;
        break;
    case LuAudio::Audio::ResultCode::Success:
        return NeoWPF::Success<void>{};
    }

    return NeoWPF::Fail<MediaError>{MediaError{code, result.Message()}};
}

}
