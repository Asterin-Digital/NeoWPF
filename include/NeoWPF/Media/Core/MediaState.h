/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

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
