/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <cstdint>
#include <string_view>

namespace NeoWPF::Diagnostics {

/*
	@summary
	Receives a formatted log message on the logger worker thread.

	@param message
	Complete message including its trailing newline.
*/
using OutputHandler = void (*)(const char* message) noexcept;

/*
	@summary
	Writes a message with informational severity.

	@param fmt
	Printf-style format string.
*/
void LogInfo(const char* fmt, ...);

/*
	@summary
	Writes a message with warning severity.

	@param fmt
	Printf-style format string.
*/
void LogWarn(const char* fmt, ...);

/*
	@summary
	Writes a message with error severity.

	@param fmt
	Printf-style format string.
*/
void LogError(const char* fmt, ...);

/*
	@summary
	Sets the destination for formatted log messages.

	@param handler
	Callback to receive messages, or `nullptr` to restore the default output.

	@details
	The callback runs on the logger worker thread.
*/
void SetOutputHandler(OutputHandler handler) noexcept;

/*
	@summary
	Stops the logger worker after it drains queued messages.
*/
void ShutdownLogger() noexcept;

/*
	@summary
	Identifies a logging severity threshold.
*/
enum class Level : uint8_t {
	Off = 0,
	Info = 1,
	Debug = 2,
	Trace = 3,
};

/*
	@summary
	Identifies a framework logging category.
*/
enum class Category : uint8_t {
	Visual = 0,
	GridLayout,
	ContentPresenter,
	Control,
	Application,
	Platform,
	HitTest,
	Virtualization,
	TextBox,
	TextRendering,
	Image,
	Http,
	XamlParser,
	ListBox,
	ComboBox,
	Popup,
	ItemsControl,
	Clipboard,
	Renderer,
	Files,
	Input,
	Count,
};

/*
	@summary
	Writes a message when its category and severity are enabled.

	@param cat
	Category associated with the message.

	@param level
	Severity of the message.

	@param fmt
	Printf-style format string.
*/
void Log(Category cat, Level level, const char* fmt, ...);

/*
	@summary
	Sets a category-specific severity threshold.

	@param cat
	Category to configure.

	@param level
	Minimum severity that is enabled.
*/
void SetLevel(Category cat, Level level) noexcept;

/*
	@summary
	Returns the configured severity threshold for a category.

	@param cat
	Category to query.

	@returns
	Configured category threshold.
*/
Level GetLevel(Category cat) noexcept;

/*
	@summary
	Checks whether a severity is enabled for a category.

	@param cat
	Category to query.

	@param level
	Severity to check.

	@returns
	`true` when the message would be logged.
*/
bool IsLevelEnabled(Category cat, Level level) noexcept;

/*
	@summary
	Sets the default severity threshold for categories without overrides.

	@param level
	Minimum severity that is enabled.
*/
void SetGlobalLevel(Level level) noexcept;

/*
	@summary
	Returns the default severity threshold.

	@returns
	Current global threshold.
*/
Level GetGlobalLevel() noexcept;

/*
	@summary
	Sets a category threshold by its name.

	@param name
	Category name, case-insensitive.

	@param level
	Minimum severity that is enabled.

	@returns
	`true` when the category name is recognized.
*/
bool SetLevelByName(std::string_view name, Level level) noexcept;

}
