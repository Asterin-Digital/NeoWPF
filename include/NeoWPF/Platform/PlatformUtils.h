/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::Platform {

/*
	@summary
	Converts a wide string to UTF-8.

	@param input
	The wide string to convert.

	@return
	The UTF-8 string, or an empty string when conversion fails.
*/
std::string WideToUtf8(const std::wstring& input);

/*
	@summary
	Converts a UTF-8 string to a wide string.

	@param input
	The UTF-8 string to convert.

	@return
	The wide string, or an empty string when conversion fails.
*/
std::wstring Utf8ToWide(const std::string& input);

/*
	@summary
	Removes whitespace from both ends of a string.

	@param value
	The string to trim.

	@return
	A trimmed copy of the string.
*/
std::string Trim(const std::string& value);

/*
	@summary
	Removes whitespace from both ends of a wide string.

	@param value
	The wide string to trim.

	@return
	A trimmed copy of the wide string.
*/
std::wstring Trim(const std::wstring& value);

/*
	@summary
	Converts a string to lowercase.

	@param value
	The string to convert.

	@return
	The lowercase string.
*/
std::string ToLower(std::string value);

/*
	@summary
	Converts a wide string to lowercase.

	@param value
	The wide string to convert.

	@return
	The lowercase wide string.
*/
std::wstring ToLower(std::wstring value);

/*
	@summary
	Converts a wide string to uppercase.

	@param value
	The wide string to convert.

	@return
	The uppercase wide string.
*/
std::wstring ToUpper(std::wstring value);

/*
	@summary
	Normalizes a file extension.

	Leading whitespace, dots, and asterisks are removed before a dot is added.
	A wildcard extension is returned as `*`.

	@param extension
	The extension to normalize, such as `txt`, `.txt`, or `*.txt`.

	@return
	The normalized extension, or an empty string for empty input.
*/
std::wstring NormalizeFileExtension(const std::wstring& extension);

/*
	@summary
	Returns whether a character is an ASCII letter.

	@param character
	The character to check.

	@return
	`true` for `A-Z` or `a-z`; otherwise `false`.
*/
bool IsAsciiAlpha(wchar_t character) noexcept;

/*
	@summary
	Returns whether a character is an ASCII digit.

	@param character
	The character to check.

	@return
	`true` for `0-9`; otherwise `false`.
*/
bool IsAsciiDigit(wchar_t character) noexcept;

/*
	@summary
	Splits a string at a delimiter.

	Empty segments are omitted from the result.

	@param value
	The string to split.

	@param delimiter
	The character that separates parts.

	@return
	The non-empty parts in their original order.
*/
std::vector<std::string> Split(const std::string& value, char delimiter);

/*
	@summary
	Parses an integer from a string.

	@param value
	The string to parse.

	@param fallback
	The value to return when parsing fails.

	@return
	The parsed integer, or `fallback` when parsing fails.
*/
int TryParseInt(const std::string& value, int fallback = 0);

/*
	@summary
	Parses a hexadecimal unsigned 32-bit integer.

	The value may optionally begin with `#` or `0x`.

	@param value
	The hexadecimal string to parse.

	@param output
	Receives the parsed value when parsing succeeds.

	@return
	`true` when parsing succeeds; otherwise `false`.
*/
bool TryParseUInt32Hex(const std::string& value, std::uint32_t& output);

}