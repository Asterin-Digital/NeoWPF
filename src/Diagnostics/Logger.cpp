#include <NeoWPF/Diagnostics/Logger.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <atomic>
#include <array>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

namespace NeoWPF::Diagnostics {

static std::atomic<uint64_t> s_logCounter{0};

static std::array<std::atomic<uint8_t>, static_cast<size_t>(Category::Count)> s_levels{};
static std::array<std::atomic<bool>, static_cast<size_t>(Category::Count)> s_levelOverrides{};
static std::atomic<uint8_t> s_globalLevel{static_cast<uint8_t>(Level::Off)};

static void Output(const char* prefix, const char* fmt, va_list ap)
{
    const uint64_t logId = ++s_logCounter;

    va_list ap_copy;
    va_copy(ap_copy, ap);
    int msg_len = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (msg_len < 0) {
        fputs("Logger error: invalid format string\n", stderr);
        return;
    }

    char idBuffer[32];
    const int idLen = snprintf(idBuffer, sizeof(idBuffer), "[%llu] ",
        static_cast<unsigned long long>(logId));

    const size_t prefix_len = strlen(prefix);
    const size_t total_len =
        static_cast<size_t>(idLen) +
        prefix_len +
        2 +
        static_cast<size_t>(msg_len) +
        2;

    std::vector<char> buffer(total_len);
    if (buffer.empty()) {
        fputs("Logger error: out of memory\n", stderr);
        return;
    }

    char* p = buffer.data();

    memcpy(p, idBuffer, idLen);
    p += idLen;

    memcpy(p, prefix, prefix_len);
    p += prefix_len;

    memcpy(p, ": ", 2);
    p += 2;

    int written = std::vsnprintf(
        p,
        static_cast<size_t>(msg_len) + 1,
        fmt,
        ap);

    p += (written >= 0 ? written : msg_len);

    *p++ = '\n';
    *p = '\0';

    constexpr size_t kDebugChunk = 4000;
    const char* chunk_start = buffer.data();
    size_t remaining = total_len - 1;

    while (remaining > 0) {
        size_t chunk_size = (remaining > kDebugChunk) ? kDebugChunk : remaining;

        char chunk_buf[kDebugChunk + 1];
        memcpy(chunk_buf, chunk_start, chunk_size);
        chunk_buf[chunk_size] = '\0';

        chunk_start += chunk_size;
        remaining -= chunk_size;
    }

#ifdef _WIN32
    const HANDLE consoleHandle = ::GetStdHandle(STD_ERROR_HANDLE);
    DWORD consoleMode = 0;
    if (consoleHandle != INVALID_HANDLE_VALUE &&
        consoleHandle != nullptr &&
        ::GetConsoleMode(consoleHandle, &consoleMode)) {
        DWORD written = 0;
        ::WriteConsoleA(consoleHandle, buffer.data(), static_cast<DWORD>(total_len - 1), &written, nullptr);
    } else {
        fputs(buffer.data(), stderr);
        fflush(stderr);
    }
    ::OutputDebugStringA(buffer.data());
#else
    fputs(buffer.data(), stderr);
    fflush(stderr);
#endif

}

[[deprecated("Use the new logger, its better")]]
void LogInfo(const char* fmt, ...)
{
    if (static_cast<Level>(s_globalLevel.load(std::memory_order_relaxed)) == Level::Off) return;
    va_list ap;
    va_start(ap, fmt);
    Output("INFO", fmt, ap);
    va_end(ap);
}

[[deprecated("Use the new logger, its better")]]
void LogWarn(const char* fmt, ...)
{
    if (static_cast<Level>(s_globalLevel.load(std::memory_order_relaxed)) == Level::Off) return;
    va_list ap;
    va_start(ap, fmt);
    Output("WARN", fmt, ap);
    va_end(ap);
}

[[deprecated("Use the new logger, its better")]]
void LogError(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    Output("ERROR", fmt, ap);
    va_end(ap);
}

bool IsLevelEnabled(Category cat, Level level) noexcept {
    const size_t index = static_cast<size_t>(cat);
    const Level catLevel = static_cast<Level>(s_levels[index].load(std::memory_order_relaxed));
    const bool hasOverride = s_levelOverrides[index].load(std::memory_order_relaxed);
    if (hasOverride) {
        return static_cast<uint8_t>(catLevel) >= static_cast<uint8_t>(level);
    }
    const Level global = static_cast<Level>(s_globalLevel.load(std::memory_order_relaxed));
    return static_cast<uint8_t>(global) >= static_cast<uint8_t>(level);
}

void SetLevel(Category cat, Level level) noexcept {
    const size_t index = static_cast<size_t>(cat);
    s_levels[index].store(static_cast<uint8_t>(level), std::memory_order_relaxed);
    s_levelOverrides[index].store(true, std::memory_order_relaxed);
}

Level GetLevel(Category cat) noexcept {
    return static_cast<Level>(s_levels[static_cast<size_t>(cat)].load(std::memory_order_relaxed));
}

void SetGlobalLevel(Level level) noexcept {
    s_globalLevel.store(static_cast<uint8_t>(level), std::memory_order_relaxed);
}

Level GetGlobalLevel() noexcept {
    return static_cast<Level>(s_globalLevel.load(std::memory_order_relaxed));
}

static std::string ToLower(std::string_view v) {
    std::string s(v);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool ParseLevel(std::string_view v, Level& out) noexcept {
    const auto s = ToLower(v);
    if (s == "off") { out = Level::Off; return true; }
    if (s == "info") { out = Level::Info; return true; }
    if (s == "debug") { out = Level::Debug; return true; }
    if (s == "trace") { out = Level::Trace; return true; }
    return false;
}

bool SetLevelByName(std::string_view name, Level level) noexcept {
    const auto n = ToLower(name);
    if (n == "visual") { SetLevel(Category::Visual, level); return true; }
    if (n == "gridlayout") { SetLevel(Category::GridLayout, level); return true; }
    if (n == "contentpresenter") { SetLevel(Category::ContentPresenter, level); return true; }
    if (n == "control") { SetLevel(Category::Control, level); return true; }
    if (n == "application") { SetLevel(Category::Application, level); return true; }
    if (n == "hittest") { SetLevel(Category::HitTest, level); return true; }
    if (n == "virtualization") { SetLevel(Category::Virtualization, level); return true; }
    if (n == "textbox") { SetLevel(Category::TextBox, level); return true; }
    if (n == "textrendering") { SetLevel(Category::TextRendering, level); return true; }
    return false;
}


void Log(Category cat, Level level, const char* fmt, ...)
{
    if (!IsLevelEnabled(cat, level)) return;
    va_list ap;
    va_start(ap, fmt);
    const char* prefix = (level == Level::Info) ? "INFO" : (level == Level::Debug) ? "DEBUG" : "TRACE";
    Output(prefix, fmt, ap);
    va_end(ap);
}

}