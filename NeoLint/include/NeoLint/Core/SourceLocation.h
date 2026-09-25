#pragma once

#include <string>

namespace neolint
{
struct SourceLocation
{
    std::string file;
    unsigned line = 0;
    unsigned column = 0;
};
}
