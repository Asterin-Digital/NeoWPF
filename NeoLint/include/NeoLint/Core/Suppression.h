#pragma once

#include <clang/Basic/SourceManager.h>

#include <string_view>

namespace neolint
{
class Suppression
{
public:
    explicit Suppression(const clang::SourceManager& sourceManager) noexcept;
    bool IsSuppressed(std::string_view ruleId, clang::SourceLocation location) const;

private:
    const clang::SourceManager& sourceManager_;
};
}
