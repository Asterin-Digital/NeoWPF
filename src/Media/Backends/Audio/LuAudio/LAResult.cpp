#include <Media/Backends/Audio/LuAudio/LAResult.h>

namespace NeoWPF::Media::LA {

MediaResult TranslateResult(const LuAudio::Audio::Result& result)
{
    if (result.Succeeded()) {
        return MediaResult::Success();
    }

    MediaResultCode code = MediaResultCode::ProcessingFailed;
    switch (result.Code()) {
    case LuAudio::Audio::ResultCode::InvalidArgument:
        code = MediaResultCode::InvalidArgument;
        break;
    case LuAudio::Audio::ResultCode::InvalidState:
        code = MediaResultCode::InvalidState;
        break;
    case LuAudio::Audio::ResultCode::BackendUnavailable:
        code = MediaResultCode::BackendUnavailable;
        break;
    case LuAudio::Audio::ResultCode::ProcessingFailed:
        code = MediaResultCode::ProcessingFailed;
        break;
    case LuAudio::Audio::ResultCode::Success:
        return MediaResult::Success();
    }

    return MediaResult::Failure(code, result.Message());
}

}
