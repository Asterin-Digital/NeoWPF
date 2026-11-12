#include <NeoWPF/Rendering/Geometry/DrawBatcher.h>

namespace NeoWPF::Rendering {

std::vector<DrawBatch> BuildDrawBatches(const CommandList& commands)
{
    std::vector<DrawBatch> batches;
    batches.reserve(commands.commands.size());

    std::size_t commandIndex = 0;
    while (commandIndex < commands.commands.size()) {
        const auto& command = commands.commands[commandIndex];
        if (command.GetKind() == CommandList::CommandKind::TexturedQuad) {
            const std::uint32_t textureId = command.GetTexturedQuad().textureId;
            const std::size_t firstCommand = commandIndex++;
            while (commandIndex < commands.commands.size()) {
                const auto& nextCommand = commands.commands[commandIndex];
                if (nextCommand.GetKind() != CommandList::CommandKind::TexturedQuad ||
                    nextCommand.GetTexturedQuad().textureId != textureId) {
                    break;
                }
                ++commandIndex;
            }
            batches.push_back({DrawBatchKind::Textured, firstCommand,
                               commandIndex - firstCommand, textureId});
            continue;
        }

        const std::size_t firstCommand = commandIndex++;
        while (commandIndex < commands.commands.size() &&
               commands.commands[commandIndex].GetKind() != CommandList::CommandKind::TexturedQuad) {
            ++commandIndex;
        }
        batches.push_back({DrawBatchKind::Solid, firstCommand,
                           commandIndex - firstCommand, 0});
    }

    return batches;
}

}