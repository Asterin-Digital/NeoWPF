#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Platform {

std::string WideToUtf8(const std::wstring& input);
std::wstring Utf8ToWide(const std::string& input);
std::string Trim(const std::string& value);
std::string ToLower(std::string value);
std::vector<std::string> Split(const std::string& value, char delimiter);
int TryParseInt(const std::string& value, int fallback = 0);
bool TryParseUInt32Hex(const std::string& value, std::uint32_t& output);

}