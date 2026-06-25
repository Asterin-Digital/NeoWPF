#include <NeoWPF/Diagnostics/Logger.h>
#include <Windows.h>
#include <cstdio>
#include <cstdarg>

namespace NeoWPF::Diagnostics {

static void Output(const char* prefix, const char* fmt, va_list ap)
{
    char buf[1024];
    int n = _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    if (n < 0) buf[sizeof(buf)-1] = '\0';
    char out[1100];
    sprintf_s(out, "%s: %s\n", prefix, buf);
    OutputDebugStringA(out);
    fputs(out, stderr);
}

void LogInfo(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Output("INFO", fmt, ap);
    va_end(ap);
}

void LogWarn(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Output("WARN", fmt, ap);
    va_end(ap);
}

void LogError(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Output("ERROR", fmt, ap);
    va_end(ap);
}

}
