#pragma once

#include <string>

namespace NeoWPF::Diagnostics {

void LogInfo(const char* fmt, ...);
void LogWarn(const char* fmt, ...);
void LogError(const char* fmt, ...);

}
