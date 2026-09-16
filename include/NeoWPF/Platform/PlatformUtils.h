/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Platform {

std::string WideToUtf8(const std::wstring& input);
std::wstring Utf8ToWide(const std::string& input);
std::string Trim(const std::string& value);
std::wstring Trim(const std::wstring& value);
std::string ToLower(std::string value);
std::wstring ToLower(std::wstring value);
std::wstring ToUpper(std::wstring value);
std::wstring NormalizeFileExtension(const std::wstring& extension);
bool IsAsciiAlpha(wchar_t character) noexcept;
bool IsAsciiDigit(wchar_t character) noexcept;
std::vector<std::string> Split(const std::string& value, char delimiter);
int TryParseInt(const std::string& value, int fallback = 0);
bool TryParseUInt32Hex(const std::string& value, std::uint32_t& output);

}