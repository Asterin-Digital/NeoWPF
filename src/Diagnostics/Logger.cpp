#include <NeoWPF/Diagnostics/Logger.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <atomic>
#include <array>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif

namespace NeoWPF::Diagnostics {

static std::atomic<uint64_t> s_logCounter{0};

static std::array<std::atomic<uint8_t>, static_cast<size_t>(Category::Count)> s_levels{};
static std::array<std::atomic<bool>, static_cast<size_t>(Category::Count)> s_levelOverrides{};
static std::atomic<uint8_t> s_globalLevel{static_cast<uint8_t>(Level::Off)};

struct LoggerState {
    std::mutex mutex;
    std::condition_variable condition;
    std::deque<std::string> messages;
    std::thread worker;
    bool stopping = false;
};

static LoggerState& GetLoggerState()
{
    static auto* state = new LoggerState();
    return *state;
}

static void WriteOutput(std::string message)
{
#ifdef _WIN32
    const HANDLE consoleHandle = ::GetStdHandle(STD_ERROR_HANDLE);
    DWORD consoleMode = 0;
    if (consoleHandle != INVALID_HANDLE_VALUE &&
        consoleHandle != nullptr &&
        ::GetConsoleMode(consoleHandle, &consoleMode)) {
        DWORD written = 0;
        ::WriteConsoleA(consoleHandle, message.data(), static_cast<DWORD>(message.size()), &written, nullptr);
    } else {
        fputs(message.c_str(), stderr);
        fflush(stderr);
    }
    ::OutputDebugStringA(message.c_str());
#else
    fputs(message.c_str(), stderr);
    fflush(stderr);
#endif
}

static void EnsureWorker()
{
    auto& state = GetLoggerState();
    static std::once_flag startFlag;
    std::call_once(startFlag, [&state] {
        state.worker = std::thread([&state] {
            for (;;) {
                std::string message;
                {
                    std::unique_lock<std::mutex> lock(state.mutex);
                    state.condition.wait(lock, [&state] {
                        return state.stopping || !state.messages.empty();
                    });
                    if (state.messages.empty() && state.stopping) {
                        return;
                    }
                    message = std::move(state.messages.front());
                    state.messages.pop_front();
                }
                WriteOutput(std::move(message));
            }
        });
        std::atexit([] { ShutdownLogger(); });
    });
}

static void Output(const char* prefix, const char* fmt, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    const int messageLength = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (messageLength < 0) {
        return;
    }

    const uint64_t logId = ++s_logCounter;
    char idBuffer[32];
    const int idLength = snprintf(idBuffer, sizeof(idBuffer), "[%llu] ",
        static_cast<unsigned long long>(logId));

    std::string message(static_cast<size_t>(idLength) + strlen(prefix) + 2 + static_cast<size_t>(messageLength) + 1, '\0');
    int written = snprintf(message.data(), message.size(), "[%llu] %s: ",
        static_cast<unsigned long long>(logId), prefix);
    if (written < 0) {
        return;
    }

    va_list formatArguments;
    va_copy(formatArguments, ap);
    const int bodyWritten = std::vsnprintf(message.data() + written,
        message.size() - static_cast<size_t>(written), fmt, formatArguments);
    va_end(formatArguments);
    if (bodyWritten < 0) {
        return;
    }
    message.resize(static_cast<size_t>(written) + static_cast<size_t>(bodyWritten));
    message.push_back('\n');

    EnsureWorker();
    auto& state = GetLoggerState();
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.stopping) {
            return;
        }
        state.messages.push_back(std::move(message));
    }
    state.condition.notify_one();
}

void ShutdownLogger() noexcept
{
    auto& state = GetLoggerState();
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.stopping = true;
    }
    state.condition.notify_all();
    if (state.worker.joinable()) {
        state.worker.join();
    }
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
    static constexpr std::pair<std::string_view, Category> kMap[] = {
        {"visual",          Category::Visual},
        {"gridlayout",      Category::GridLayout},
        {"contentpresenter",Category::ContentPresenter},
        {"control",         Category::Control},
        {"application",     Category::Application},
        {"hittest",         Category::HitTest},
        {"virtualization",  Category::Virtualization},
        {"textbox",         Category::TextBox},
        {"textrendering",   Category::TextRendering},
        {"xamlparser",      Category::XamlParser},
        {"listbox",         Category::ListBox},
        {"combobox",        Category::ComboBox},
        {"popup",           Category::Popup},
        {"itemscontrol",    Category::ItemsControl},
        {"renderer",        Category::Renderer},
        {"files",           Category::Files},
    };

    const auto n = ToLower(name);
    for (const auto& [key, cat] : kMap) {
        if (n == key) {
            SetLevel(cat, level);
            return true;
        }
    }
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