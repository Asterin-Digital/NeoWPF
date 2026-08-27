#include <NeoWPF/Diagnostics/CrashHandler.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>

#include <Windows.h>
#include <cstring>
#include <string>
#include <vector>

namespace NeoWPF::Platform {
namespace {

std::wstring ModulePath()
{
    wchar_t path[MAX_PATH] = {};
    if (::GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) {
        return L"<unknown>";
    }
    return path;
}

LONG WINAPI UnhandledExceptionFilterImpl(EXCEPTION_POINTERS* exceptionInfo)
{
    NeoWPF::Diagnostics::CrashContext context;
    context.category = NeoWPF::Diagnostics::CrashCategory::UnhandledException;
    context.exceptionCode = exceptionInfo && exceptionInfo->ExceptionRecord
        ? exceptionInfo->ExceptionRecord->ExceptionCode : 0;
    context.exceptionAddress = exceptionInfo && exceptionInfo->ExceptionRecord
        ? reinterpret_cast<std::uintptr_t>(exceptionInfo->ExceptionRecord->ExceptionAddress) : 0;
    context.threadId = ::GetCurrentThreadId();
    context.exceptionType = L"UnhandledException";
    context.source = L"NeoWPF.UnhandledException";
    context.modulePath = ModulePath();
    context.description = L"Unhandled exception was raised before the application could continue.";
    std::vector<NeoWPF::Diagnostics::CallStackEntry> entries(32);
    const int count = NeoWPF::Diagnostics::CaptureCallStack(entries.data(), static_cast<int>(entries.size()), 1);
    for (int index = 0; index < count; ++index) {
        if (entries[index].symbol[0] != '\0') {
            context.callStack.emplace_back(entries[index].symbol, entries[index].symbol + std::strlen(entries[index].symbol));
        } else {
            context.callStack.emplace_back(L"<unknown>");
        }
    }
    context.handled = false;
    NeoWPF::Diagnostics::ReportCrash(context);
    return EXCEPTION_EXECUTE_HANDLER;
}

}

void InstallCrashHandlerWindows() noexcept
{
    ::SetUnhandledExceptionFilter(UnhandledExceptionFilterImpl);
}

void UninstallCrashHandlerWindows() noexcept
{
    ::SetUnhandledExceptionFilter(nullptr);
}

std::wstring CurrentModulePathWindows()
{
    return ModulePath();
}

std::uint32_t CurrentThreadIdWindows() noexcept
{
    return static_cast<std::uint32_t>(::GetCurrentThreadId());
}

}