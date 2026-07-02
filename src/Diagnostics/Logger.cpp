#include <NeoWPF/Diagnostics/Logger.h>
#include <Windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <new>

namespace NeoWPF::Diagnostics {

static void Output(const char* prefix, const char* fmt, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int msg_len = _vscprintf(fmt, ap_copy);
    va_end(ap_copy);

    if (msg_len < 0) {
        OutputDebugStringA("Logger error: invalid format string\n");
        fputs("Logger error: invalid format string\n", stderr);
        return;
    }

    const size_t prefix_len = strlen(prefix);
    const size_t total_len  = prefix_len + 2 + static_cast<size_t>(msg_len) + 2;

    char* buffer = new (std::nothrow) char[total_len];
    if (!buffer) {
        OutputDebugStringA("Logger error: out of memory\n");
        fputs("Logger error: out of memory\n", stderr);
        return;
    }

    char* p = buffer;
    memcpy(p, prefix, prefix_len);
    p += prefix_len;
    memcpy(p, ": ", 2);
    p += 2;

    int written = _vsnprintf_s(p, static_cast<size_t>(msg_len) + 1,
                               _TRUNCATE, fmt, ap);
    p += (written >= 0 ? written : msg_len);
    *p++ = '\n';
    *p   = '\0';

    constexpr size_t kDebugChunk = 4000;
    const char* chunk_start = buffer;
    size_t remaining = total_len - 1;

    while (remaining > 0) {
        size_t chunk_size = (remaining > kDebugChunk) ? kDebugChunk : remaining;

        char chunk_buf[kDebugChunk + 1];
        memcpy(chunk_buf, chunk_start, chunk_size);
        chunk_buf[chunk_size] = '\0';
        OutputDebugStringA(chunk_buf);

        chunk_start += chunk_size;
        remaining   -= chunk_size;
    }

    fputs(buffer, stderr);

    delete[] buffer;
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