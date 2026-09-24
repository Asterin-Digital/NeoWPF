/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NeoWPF::Diagnostics {

enum class CrashCategory {
    Unknown,
    UnhandledException,
    Terminate,
    Signal
};

struct CrashContext {
    CrashCategory category = CrashCategory::Unknown;
    std::wstring exceptionType;
    std::wstring message;
    std::wstring source;
    std::wstring modulePath;
    std::wstring description;
    std::uint32_t exceptionCode = 0;
    std::uintptr_t exceptionAddress = 0;
    std::uint32_t threadId = 0;
    std::vector<std::wstring> callStack;
    bool handled = false;
};

using CrashHandlerCallback = std::function<void(const CrashContext&)>;

void InitializeCrashHandler();
void ShutdownCrashHandler();

void RegisterCrashHandler(const CrashHandlerCallback& handler);
void RegisterExceptionHandler(const std::wstring& exceptionType, const CrashHandlerCallback& handler);
void UnregisterCrashHandler(const CrashHandlerCallback& handler);
void UnregisterExceptionHandler(const std::wstring& exceptionType, const CrashHandlerCallback& handler);

void ReportCrash(const CrashContext& context);

}