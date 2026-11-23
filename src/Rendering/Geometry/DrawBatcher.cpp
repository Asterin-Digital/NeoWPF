#include <NeoWPF/Rendering/Geometry/DrawBatcher.h>

namespace NeoWPF::Rendering {
namespace {

Paint GetCommandPaint(const CommandList::Command& command) noexcept
{
    switch (command.GetKind()) {
    case CommandList::CommandKind::Rect:
        return command.GetRect().paint;
    case CommandList::CommandKind::RoundedRect:
        return command.GetRoundedRect().paint;
    case CommandList::CommandKind::Arc:
        return command.GetArc().paint;
    case CommandList::CommandKind::TexturedQuad:
        return command.GetTexturedQuad().tint;
    }
    return Paint::Solid(0);
}

}

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
                               commandIndex - firstCommand, textureId, 0});
            continue;
        }

        const std::size_t firstCommand = commandIndex++;
        const Paint firstPaint = GetCommandPaint(command);
        const DrawBatchKind kind = firstPaint.kind == Paint::Kind::LinearGradient
            ? DrawBatchKind::LinearGradient
            : firstPaint.kind == Paint::Kind::RadialGradient
                ? DrawBatchKind::RadialGradient
                : DrawBatchKind::Solid;
        while (commandIndex < commands.commands.size()) {
            const auto& nextCommand = commands.commands[commandIndex];
            if (nextCommand.GetKind() == CommandList::CommandKind::TexturedQuad) {
                break;
            }
            const Paint nextPaint = GetCommandPaint(nextCommand);
            const DrawBatchKind nextKind = nextPaint.kind == Paint::Kind::LinearGradient
                ? DrawBatchKind::LinearGradient
                : nextPaint.kind == Paint::Kind::RadialGradient
                    ? DrawBatchKind::RadialGradient
                    : DrawBatchKind::Solid;
            if (nextKind != kind ||
                (kind == DrawBatchKind::LinearGradient || kind == DrawBatchKind::RadialGradient) &&
                    nextPaint.gradientIndex != firstPaint.gradientIndex) {
                break;
            }
            ++commandIndex;
        }
        batches.push_back({kind, firstCommand, commandIndex - firstCommand, 0,
                           kind == DrawBatchKind::Solid ? 0 : firstPaint.gradientIndex});
    }

    return batches;
}

}