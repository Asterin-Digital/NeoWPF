#pragma once

#include <string>

namespace NeoPM::Utils {
    std::string WideToUtf8(const std::wstring& value);
    std::wstring Utf8ToWide(const std::string& value);
}