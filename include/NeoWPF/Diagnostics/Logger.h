/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <cstdint>
#include <string_view>

namespace NeoWPF::Diagnostics {

// you can still use these but if u want better debugging i recommend adding a Category for what ur debugging
void LogInfo(const char* fmt, ...);
void LogWarn(const char* fmt, ...);
void LogError(const char* fmt, ...);
void ShutdownLogger() noexcept;

enum class Level : uint8_t {
	Off = 0,
	Info = 1,
	Debug = 2,
	Trace = 3,
};

enum class Category : uint8_t {
	Visual = 0,
	GridLayout,
	ContentPresenter,
	Control,
	Application,
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
	Count,
};

void Log(Category cat, Level level, const char* fmt, ...);

void SetLevel(Category cat, Level level) noexcept;
Level GetLevel(Category cat) noexcept;
bool IsLevelEnabled(Category cat, Level level) noexcept;

void SetGlobalLevel(Level level) noexcept;
Level GetGlobalLevel() noexcept;

bool SetLevelByName(std::string_view name, Level level) noexcept;

}
