#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace NeoWPF::Diagnostics {
namespace {

struct HandlerEntry {
    CrashHandlerCallback callback;
};

struct ExceptionHandlerEntry {
    std::wstring exceptionType;
    CrashHandlerCallback callback;
};

std::once_flag s_initFlag;
std::once_flag s_shutdownFlag;
std::mutex s_handlerMutex;
std::vector<HandlerEntry> s_handlers;
std::vector<ExceptionHandlerEntry> s_exceptionHandlers;
std::atomic<bool> s_initialized{false};
std::atomic<bool> s_shutdown{false};

std::vector<std::wstring> CaptureCallStackText()
{
    std::vector<std::wstring> output;
    std::vector<CallStackEntry> entries(32);
    const int count = CaptureCallStack(entries.data(), static_cast<int>(entries.size()), 1);
    for (int i = 0; i < count; ++i) {
        std::wstring symbol;
        if (entries[i].symbol[0] != '\0') {
            symbol.assign(entries[i].symbol, entries[i].symbol + std::strlen(entries[i].symbol));
        } else {
            symbol = L"<unknown>";
        }
        output.emplace_back(std::move(symbol));
    }
    return output;
}

void EmitCrashReport(const CrashContext& context)
{
    if (!context.message.empty()) {
        LogError("Crash report: %s", NeoWPF::Platform::WideToUtf8(context.message).c_str());
    } else {
        LogError("Crash report: %s", NeoWPF::Platform::WideToUtf8(context.description).c_str());
    }

    if (context.exceptionCode != 0) {
        LogError("Exception code: 0x%08X", context.exceptionCode);
    }

    if (!context.exceptionType.empty()) {
        LogError("Exception type: %s", NeoWPF::Platform::WideToUtf8(context.exceptionType).c_str());
    }

    if (!context.source.empty()) {
        LogError("Source: %s", NeoWPF::Platform::WideToUtf8(context.source).c_str());
    }

    if (!context.modulePath.empty()) {
        LogError("Module: %s", NeoWPF::Platform::WideToUtf8(context.modulePath).c_str());
    }

    if (!context.callStack.empty()) {
        LogError("Call stack:");
        for (const auto& frame : context.callStack) {
            LogError("  %s", NeoWPF::Platform::WideToUtf8(frame).c_str());
        }
    }
}

void DispatchCrash(const CrashContext& context)
{
    std::vector<HandlerEntry> handlers;
    std::vector<ExceptionHandlerEntry> exceptionHandlers;
    {
        std::lock_guard<std::mutex> lock(s_handlerMutex);
        handlers = s_handlers;
        exceptionHandlers = s_exceptionHandlers;
    }

    for (const auto& handler : handlers) {
        if (handler.callback) {
            handler.callback(context);
        }
    }

    for (const auto& handler : exceptionHandlers) {
        if (handler.callback && handler.exceptionType == context.exceptionType) {
            handler.callback(context);
        }
    }

    EmitCrashReport(context);
}

void TerminateHandler()
{
    CrashContext context;
    context.category = CrashCategory::Terminate;
    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        context.threadId = platform->CurrentThreadId();
        context.modulePath = platform->CurrentModulePath();
    }
    context.exceptionType = L"std::terminate";
    context.source = L"NeoWPF.Terminate";
    context.description = L"The process was terminated by std::terminate().";
    context.callStack = CaptureCallStackText();
    context.handled = false;
    DispatchCrash(context);
    std::abort();
}

void SignalHandler(int signal)
{
    CrashContext context;
    context.category = CrashCategory::Signal;
    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        context.threadId = platform->CurrentThreadId();
        context.modulePath = platform->CurrentModulePath();
    }
    context.exceptionType = L"signal";
    context.message = std::to_wstring(signal);
    context.source = L"NeoWPF.Signal";
    context.description = L"The process received a fatal signal.";
    context.callStack = CaptureCallStackText();
    context.handled = false;
    DispatchCrash(context);
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void InstallHandlers()
{
    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        platform->InstallCrashHandler();
    }
    std::set_terminate(TerminateHandler);
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGFPE, SignalHandler);
    std::signal(SIGILL, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
}

void UninstallHandlers()
{
    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        platform->UninstallCrashHandler();
    }
    std::signal(SIGABRT, SIG_DFL);
    std::signal(SIGFPE, SIG_DFL);
    std::signal(SIGILL, SIG_DFL);
    std::signal(SIGSEGV, SIG_DFL);
}

} // namespace

void InitializeCrashHandler()
{
    std::call_once(s_initFlag, []() {
        s_initialized.store(true, std::memory_order_release);
        InstallHandlers();
    });
}

void ShutdownCrashHandler()
{
    std::call_once(s_shutdownFlag, []() {
        if (s_initialized.load(std::memory_order_acquire)) {
            UninstallHandlers();
        }
        s_initialized.store(false, std::memory_order_release);
        s_shutdown.store(true, std::memory_order_release);
    });
}

void RegisterCrashHandler(const CrashHandlerCallback& handler)
{
    if (!handler) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_handlerMutex);
    s_handlers.emplace_back(HandlerEntry{handler});
}

void RegisterExceptionHandler(const std::wstring& exceptionType, const CrashHandlerCallback& handler)
{
    if (exceptionType.empty() || !handler) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_handlerMutex);
    s_exceptionHandlers.emplace_back(ExceptionHandlerEntry{exceptionType, handler});
}

void UnregisterCrashHandler(const CrashHandlerCallback& handler)
{
    (void)handler;
    std::lock_guard<std::mutex> lock(s_handlerMutex);
    s_handlers.clear();
}

void UnregisterExceptionHandler(const std::wstring& exceptionType, const CrashHandlerCallback& handler)
{
    (void)handler;
    std::lock_guard<std::mutex> lock(s_handlerMutex);
    if (exceptionType.empty()) {
        s_exceptionHandlers.clear();
        return;
    }

    s_exceptionHandlers.erase(std::remove_if(s_exceptionHandlers.begin(), s_exceptionHandlers.end(), [&](const ExceptionHandlerEntry& entry) {
        return entry.exceptionType == exceptionType;
    }), s_exceptionHandlers.end());
}

void ReportCrash(const CrashContext& context)
{
    DispatchCrash(context);
}

}
