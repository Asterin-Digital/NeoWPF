#pragma once

#include <string>

namespace NeoWPF::Platform {

std::string WideToUtf8(const std::wstring& input);
std::wstring Utf8ToWide(const std::string& input);

}