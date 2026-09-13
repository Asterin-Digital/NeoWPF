#pragma once

namespace NeoWPF::Media {

enum class MediaState {
    Closed,
    Opening,
    Stopped,
    Playing,
    Paused,
    Ended,
    Failed,
};

}
