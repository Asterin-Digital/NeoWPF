#include <NeoWPF/Platform/Android/AWindowing.h>
#include <NeoWPF/Platform/Android/AWindow.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Controls/Window.h>

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include <android/looper.h>

#include <algorithm>
#include <chrono>
#include <limits>
#include <utility>
#include <unistd.h>

namespace NeoWPF::Platform::Android {
namespace {

const char* VulkanInstanceExtensions[] = {
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
};

bool CreateVulkanSurface(void*, void* instance, void* window, void*, void* outSurface) noexcept
{
    const VkAndroidSurfaceCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        static_cast<ANativeWindow*>(window),
    };
    const VkResult result = vkCreateAndroidSurfaceKHR(
        static_cast<VkInstance>(instance), &createInfo, nullptr,
        static_cast<VkSurfaceKHR*>(outSurface));
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
        NeoWPF::Diagnostics::Level::Trace,
        "[Android/Surface] Vulkan factory: instance=%p nativeWindow=%p result=%d surfaceNull=%d",
        instance, window, static_cast<int>(result),
        *static_cast<VkSurfaceKHR*>(outSurface) == VK_NULL_HANDLE ? 1 : 0);
    return result == VK_SUCCESS;
}

}

AWindowing::~AWindowing()
{
    Stop();
}

void* AWindowing::CreateWindow(const wchar_t*, int, int,
                               std::shared_ptr<NeoWPF::UI::Window> windowHost)
{
    if (!m_windows.empty() || !m_activityAlive.load(std::memory_order_acquire)) {
        return nullptr;
    }

    auto window = std::make_unique<AWindow>();
    void* const handle = window.get();
    NeoWPF::Core::WindowSession session;
    session.renderSurface.nativeWindowHandle = handle;
    session.renderSurface.Invalidate();
    session.windowHost = windowHost ? std::move(windowHost) : std::make_shared<NeoWPF::UI::Window>();
    {
        auto& app = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        session.lifecycle.Deactivate();
        app.windowSessions.push_back(std::move(session));
        app.hasCreatedWindow = true;
    }
    m_windows.push_back(std::move(window));
    return handle;
}

bool AWindowing::GetNativeWindowInfo(
    void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept
{
    outInfo = {};
    const auto* window = FindWindow(nativeWindowHandle);
    ANativeWindow* const nativeWindow = window ? window->NativeWindow() : nullptr;
    if (!nativeWindow || !m_activityAlive.load(std::memory_order_acquire)) {
        return false;
    }

    outInfo.system = NeoWPF::Contracts::NativeWindowSystem::Android;
    outInfo.surfaceAlpha = NeoWPF::Contracts::SurfaceAlpha::Opaque;
    outInfo.window = nativeWindow;
    outInfo.vulkanInstanceExtensions = VulkanInstanceExtensions;
    outInfo.vulkanInstanceExtensionCount = std::size(VulkanInstanceExtensions);
    outInfo.createVulkanSurface = &CreateVulkanSurface;
    return true;
}

bool AWindowing::IsWindowAlive(void* nativeWindowHandle) const noexcept
{
    return FindWindow(nativeWindowHandle) != nullptr &&
        m_activityAlive.load(std::memory_order_acquire);
}

void AWindowing::GetWindowSize(void* nativeWindowHandle,
                               unsigned int& outWidth, unsigned int& outHeight) const noexcept
{
    outWidth = 0;
    outHeight = 0;
    if (const auto* window = FindWindow(nativeWindowHandle)) {
        window->GetSize(outWidth, outHeight);
    }
}

void AWindowing::ResizeWindow(void*, int, int) noexcept
{
}

void AWindowing::ShowWindow(void*) noexcept
{
}

void AWindowing::HideWindow(void*) noexcept
{
}

void AWindowing::CloseWindow(void* nativeWindowHandle) noexcept
{
    const auto iterator = std::find_if(m_windows.begin(), m_windows.end(),
        [nativeWindowHandle](const auto& window) { return window.get() == nativeWindowHandle; });
    if (iterator == m_windows.end()) {
        return;
    }

    NeoWPF::Core::CloseWindow(nativeWindowHandle);
    (*iterator)->SetNativeWindow(nullptr);
    m_windows.erase(iterator);
    if (m_windows.empty()) {
        QueueActivityDestroyed();
    }
}

void AWindowing::InvalidateWindow(void*) noexcept
{
    WakeEventPump();
}

bool AWindowing::PumpMessages(std::chrono::milliseconds timeout)
{
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        if (m_stopped) {
            return false;
        }
        if (!m_looper) {
            m_looper = ALooper_prepare(0);
        }
        if (!m_looper) {
            return false;
        }
    }

    const auto boundedTimeout = std::clamp(
        timeout, std::chrono::milliseconds::zero(),
        std::chrono::milliseconds(std::numeric_limits<int>::max()));
    ALooper_pollOnce(static_cast<int>(boundedTimeout.count()), nullptr, nullptr, nullptr);
    {
        ProcessEvents();
    }
    return m_activityAlive.load(std::memory_order_acquire) && !m_windows.empty();
}

int AWindowing::ExitCode() const noexcept
{
    return m_exitCode.load(std::memory_order_acquire);
}

void AWindowing::RequestQuit(int exitCode) noexcept
{
    m_exitCode.store(exitCode, std::memory_order_release);
    QueueActivityDestroyed();
}

void AWindowing::ShowFatalError(const std::wstring&, const std::wstring& message) noexcept
{
    NeoWPF::Diagnostics::LogError("%s", NeoWPF::Platform::WideToUtf8(message).c_str());
}

void AWindowing::SetConsoleVisible(bool) noexcept
{
}

bool AWindowing::IsConsoleVisible() const noexcept
{
    return false;
}

void AWindowing::QueueSurfaceAvailable(ANativeWindow* nativeWindow,
                                      unsigned int width, unsigned int height) noexcept
{
    if (!nativeWindow) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        if (!m_activityAlive.load(std::memory_order_acquire)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
                NeoWPF::Diagnostics::Level::Trace,
                "[Android/Surface] rejected native window: nativeWindow=%p extent=%ux%u activityAlive=0 queued=%zu",
                nativeWindow, width, height, m_events.size());
            ANativeWindow_release(nativeWindow);
            return;
        }
        Event event;
        event.type = EventType::SurfaceAvailable;
        event.nativeWindow = nativeWindow;
        event.width = width;
        event.height = height;
        m_events.push_back(std::move(event));
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
            NeoWPF::Diagnostics::Level::Trace,
            "[Android/Surface] queued native window: nativeWindow=%p extent=%ux%u activityAlive=%d queued=%zu",
            nativeWindow, width, height,
            m_activityAlive.load(std::memory_order_relaxed) ? 1 : 0, m_events.size());
    }
    WakeEventPump();
}

void AWindowing::QueueSurfaceDestroyed() noexcept
{
    auto completion = std::make_shared<std::promise<void>>();
    auto finished = completion->get_future();
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        if (m_stopped) {
            return;
        }
        Event event;
        event.type = EventType::SurfaceDestroyed;
        event.completion = std::move(completion);
        m_events.push_back(std::move(event));
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
            NeoWPF::Diagnostics::Level::Trace,
            "[Android/Surface] queued destroy: stopped=%d queued=%zu",
            m_stopped ? 1 : 0, m_events.size());
    }
    WakeEventPump();
    finished.wait();
}

void AWindowing::QueueInputQueueCreated(AInputQueue* inputQueue) noexcept
{
    if (!inputQueue) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "T2 QueueInputQueueCreated queue=%p stopped=%d tid=%d",
            inputQueue, m_stopped, gettid());
        if (m_stopped) {
            return;
        }
        Event event;
        event.type = EventType::InputQueueCreated;
        event.inputQueue = inputQueue;
        m_events.push_back(std::move(event));
    }
    WakeEventPump();
}

void AWindowing::QueueInputQueueDestroyed(AInputQueue* inputQueue) noexcept
{
    if (!inputQueue) {
        return;
    }
    auto completion = std::make_shared<std::promise<void>>();
    auto finished = completion->get_future();
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        if (m_stopped) {
            return;
        }
        Event event;
        event.type = EventType::InputQueueDestroyed;
        event.inputQueue = inputQueue;
        event.completion = std::move(completion);
        m_events.push_back(std::move(event));
    }
    WakeEventPump();
    finished.wait();
}

void AWindowing::QueueActivityDestroyed() noexcept
{
    m_activityAlive.store(false, std::memory_order_release);
    WakeEventPump();
}

bool AWindowing::HasSurface() const noexcept
{
    return !m_windows.empty() && m_windows.front()->NativeWindow() != nullptr;
}

bool AWindowing::WaitForSurface(std::chrono::seconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (m_activityAlive.load(std::memory_order_acquire) && !HasSurface()) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return false;
        }
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        PumpMessages(std::min(remaining, std::chrono::milliseconds(50)));
    }
    return HasSurface();
}

void AWindowing::Stop() noexcept
{
    m_activityAlive.store(false, std::memory_order_release);
    std::deque<Event> pendingEvents;
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        if (m_stopped) {
            return;
        }
        StopInputQueue();
        m_looper = nullptr;
        m_stopped = true;
        pendingEvents.swap(m_events);
    }
    for (auto& event : pendingEvents) {
        if (event.nativeWindow) {
            ANativeWindow_release(event.nativeWindow);
        }
    }
    for (auto& window : m_windows) {
        window->SetNativeWindow(nullptr);
        window->SetSize(0, 0);
    }
    for (auto& event : pendingEvents) {
        if (event.completion) {
            event.completion->set_value();
        }
    }
}

AWindow* AWindowing::FindWindow(void* nativeWindowHandle) noexcept
{
    const auto iterator = std::find_if(m_windows.begin(), m_windows.end(),
        [nativeWindowHandle](const auto& window) { return window.get() == nativeWindowHandle; });
    return iterator == m_windows.end() ? nullptr : iterator->get();
}

const AWindow* AWindowing::FindWindow(void* nativeWindowHandle) const noexcept
{
    const auto iterator = std::find_if(m_windows.begin(), m_windows.end(),
        [nativeWindowHandle](const auto& window) { return window.get() == nativeWindowHandle; });
    return iterator == m_windows.end() ? nullptr : iterator->get();
}

void AWindowing::ProcessEvents() noexcept
{
    for (;;) {
        Event event;
        {
            std::lock_guard<std::mutex> lock(m_eventMutex);
            if (m_events.empty()) {
                return;
            }
            event = std::move(m_events.front());
            m_events.pop_front();
        }

        switch (event.type) {
        case EventType::SurfaceAvailable:
            ProcessSurfaceAvailable(event);
            break;
        case EventType::SurfaceDestroyed:
            ProcessSurfaceDestroyed();
            break;
        case EventType::InputQueueCreated:
        case EventType::InputQueueDestroyed:
            ProcessInputQueueEvent(event);
            break;
        }
        if (event.completion) {
            event.completion->set_value();
        }
    }
}

void AWindowing::WakeEventPump() noexcept
{
    std::lock_guard<std::mutex> lock(m_eventMutex);
    if (m_looper) {
        ALooper_wake(m_looper);
    }
}

void AWindowing::ProcessSurfaceAvailable(Event& event) noexcept
{
    AWindow* const window = m_windows.empty() ? nullptr : m_windows.front().get();
    if (!window || !event.nativeWindow) {
        if (event.nativeWindow) {
            ANativeWindow_release(event.nativeWindow);
            event.nativeWindow = nullptr;
        }
        return;
    }

    ANativeWindow* const previous = window->NativeWindow();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
        NeoWPF::Diagnostics::Level::Trace,
        "[Android/Surface] applying native window: windowHandle=%p nativeWindow=%p previous=%p extent=%ux%u replacement=%d",
        window, event.nativeWindow, previous, event.width, event.height,
        previous && previous != event.nativeWindow ? 1 : 0);
    if (previous && previous != event.nativeWindow) {
        NeoWPF::Core::ReleaseWindowSurface(window);
    }
    window->SetNativeWindow(event.nativeWindow);
    event.nativeWindow = nullptr;
    window->SetSize(event.width, event.height);

    if (!previous || previous != window->NativeWindow()) {
        if (!NeoWPF::Core::AttachWindowSurface(window)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
                NeoWPF::Diagnostics::Level::Trace,
                "[Android/Surface] native window attach failed: windowHandle=%p nativeWindow=%p extent=%ux%u",
                window, window->NativeWindow(), event.width, event.height);
            window->SetNativeWindow(nullptr);
            window->SetSize(0, 0);
            return;
        }
        NeoWPF::Core::SetInputRoot(window);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
            NeoWPF::Diagnostics::Level::Trace,
            "[Android/Surface] native window installed: windowHandle=%p nativeWindow=%p extent=%ux%u",
            window, window->NativeWindow(), event.width, event.height);
    }
    InvalidateWindow(window);
}

void AWindowing::ProcessSurfaceDestroyed() noexcept
{
    if (m_windows.empty()) {
        return;
    }
    AWindow* const window = m_windows.front().get();
    if (window->NativeWindow()) {
        unsigned int width = 0;
        unsigned int height = 0;
        window->GetSize(width, height);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Platform,
            NeoWPF::Diagnostics::Level::Trace,
            "[Android/Surface] removing native window: windowHandle=%p nativeWindow=%p extent=%ux%u",
            window, window->NativeWindow(), width, height);
        NeoWPF::Core::ReleaseWindowSurface(window);
        window->SetNativeWindow(nullptr);
        window->SetSize(0, 0);
    }
}

}