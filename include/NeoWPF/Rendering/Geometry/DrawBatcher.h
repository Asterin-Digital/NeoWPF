/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Renderer.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace NeoWPF::Rendering {

/*
    @summary
    Identifies the pipeline class used by a contiguous draw batch.

    @internal
*/
enum class DrawBatchKind : std::uint8_t {
    Solid,
    Textured,
};

/*
    @summary
    Describes a contiguous range of compatible render commands.

    @internal
*/
struct DrawBatch {
    DrawBatchKind kind;
    std::size_t firstCommand;
    std::size_t commandCount;
    std::uint32_t textureId;
};

/*
    @summary
    Groups adjacent commands without changing their order.

    @param commands
    The command list to batch.

    @returns
    Contiguous batches in painter's order.

    @internal
*/
std::vector<DrawBatch> BuildDrawBatches(const CommandList& commands);

}