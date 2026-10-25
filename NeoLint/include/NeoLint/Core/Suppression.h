#pragma once

#include <clang/Basic/SourceManager.h>

#include <cstddef>
#include <string_view>

namespace neolint
{
class Suppression
{
public:
    explicit Suppression(const clang::SourceManager& sourceManager) noexcept;
    bool IsSuppressed(std::string_view ruleId, clang::SourceLocation location) const;
    static bool IsSuppressed(std::string_view ruleId, std::string_view source,
                             std::size_t lineNumber);

private:
    const clang::SourceManager& sourceManager_;
};
}
