#pragma once

#include <string>
#include <cstdint>
#include <string_view>

namespace NeoWPF::Diagnostics {

void LogInfo(const char* fmt, ...);
void LogWarn(const char* fmt, ...);
void LogError(const char* fmt, ...);

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
	TextBox,
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
