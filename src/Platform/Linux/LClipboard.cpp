#include <NeoWPF/Platform/Linux/LClipboard.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/Linux/LWindowing.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <utility>

namespace NeoWPF::Platform::Linux {
namespace {

constexpr const char* Utf8MimeType = "text/plain;charset=utf-8";
constexpr const char* PlainMimeType = "text/plain";

bool FlushSucceeded(int result, int error) noexcept
{
    return result == 0 || (result < 0 && error == EAGAIN);
}

void HandleDataSourceTarget(void* data, wl_data_source*, const char*)
{
    static_cast<LClipboard*>(data);
}

void HandleDataSourceSend(void* data, wl_data_source*, const char*, int fd)
{
    auto* clipboard = static_cast<LClipboard*>(data);
    const auto text = NeoWPF::Platform::WideToUtf8(clipboard->OwnedText());
    const char* cursor = text.data();
    std::size_t remaining = text.size();
    while (remaining != 0) {
        const auto written = write(fd, cursor, remaining);
        if (written <= 0) {
            break;
        }
        cursor += written;
        remaining -= static_cast<std::size_t>(written);
    }
    close(fd);
}

void HandleDataSourceCancelled(void* data, wl_data_source* source)
{
    auto* clipboard = static_cast<LClipboard*>(data);
    clipboard->SourceCancelled(source);
}

const wl_data_source_listener DataSourceListener = {
    &HandleDataSourceTarget,
    &HandleDataSourceSend,
    &HandleDataSourceCancelled,
    nullptr,
    nullptr,
    nullptr,
};

void HandleDataDeviceOffer(void* data, wl_data_device*, wl_data_offer* offer)
{
    static_cast<LClipboard*>(data)->OfferCreated(offer);
}

void HandleDataDeviceSelection(void* data, wl_data_device*, wl_data_offer* offer)
{
    static_cast<LClipboard*>(data)->SelectionChanged(offer);
}

const wl_data_device_listener DataDeviceListener = {
    &HandleDataDeviceOffer,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &HandleDataDeviceSelection,
};

void HandleOfferMime(void* data, wl_data_offer* offer, const char* mimeType)
{
    static_cast<LClipboard*>(data)->OfferMimeType(offer, mimeType);
}

const wl_data_offer_listener DataOfferListener = {
    &HandleOfferMime,
    nullptr,
    nullptr,
};

}

LClipboard::LClipboard(LWindowing& windowing) noexcept
    : m_windowing(windowing)
{
    m_windowing.AttachClipboard(this);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] Attached clipboard");
}

LClipboard::~LClipboard()
{
    if (m_pendingReadFd >= 0) {
        close(m_pendingReadFd);
    }
    if (m_source) {
        wl_data_source_destroy(m_source);
    }
    if (m_selectionOffer) {
        wl_data_offer_destroy(m_selectionOffer);
    }
    if (m_pendingOffer && m_pendingOffer != m_selectionOffer) {
        wl_data_offer_destroy(m_pendingOffer);
    }
    if (m_dataDevice) {
        wl_data_device_destroy(m_dataDevice);
    }
    m_windowing.DetachClipboard(this);
}

bool LClipboard::EnsureDevice() noexcept
{
    if (m_dataDevice) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Debug, "[LinuxClipboard] Data device already available");
        return true;
    }
    if (!m_windowing.Connect() || !m_windowing.DataDeviceManager() || !m_windowing.Seat()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] Cannot initialize data device: connected=%d manager=%p seat=%p",
            m_windowing.IsConnected(),
            static_cast<void*>(m_windowing.DataDeviceManager()),
            static_cast<void*>(m_windowing.Seat()));
        return false;
    }
    m_dataDevice = wl_data_device_manager_get_data_device(
        m_windowing.DataDeviceManager(), m_windowing.Seat());
    if (!m_dataDevice || wl_data_device_add_listener(m_dataDevice, &DataDeviceListener, this) != 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] Failed to create or register data device: device=%p",
            static_cast<void*>(m_dataDevice));
        if (m_dataDevice) {
            wl_data_device_destroy(m_dataDevice);
            m_dataDevice = nullptr;
        }
        return false;
    }
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] Data device initialized: %p",
        static_cast<void*>(m_dataDevice));
    return true;
}

bool LClipboard::SetText(const std::wstring& text)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClipboard] SetText requested: characters=%zu serial=%u",
        text.size(), m_windowing.LastInputSerial());
    if (!EnsureDevice() || m_windowing.LastInputSerial() == 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] SetText rejected: device=%p serial=%u",
            static_cast<void*>(m_dataDevice), m_windowing.LastInputSerial());
        return false;
    }

    if (m_source) {
        wl_data_source_destroy(m_source);
        m_source = nullptr;
    }
    m_source = wl_data_device_manager_create_data_source(m_windowing.DataDeviceManager());
    if (!m_source || wl_data_source_add_listener(m_source, &DataSourceListener, this) != 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] Failed to create or register data source: source=%p",
            static_cast<void*>(m_source));
        if (m_source) {
            wl_data_source_destroy(m_source);
            m_source = nullptr;
        }
        return false;
    }

    m_ownedText = text;
    wl_data_source_offer(m_source, Utf8MimeType);
    wl_data_source_offer(m_source, PlainMimeType);
    wl_data_device_set_selection(m_dataDevice, m_source, m_windowing.LastInputSerial());
    const int flushResult = wl_display_flush(m_windowing.Display());
    const int flushError = errno;
    const bool flushed = FlushSucceeded(flushResult, flushError);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClipboard] SetText %s: source=%p formats='%s','%s' flush=%d errno=%d",
        flushed ? "succeeded" : "failed to flush display",
        static_cast<void*>(m_source), Utf8MimeType, PlainMimeType,
        flushResult, flushError);
    return flushed;
}

void LClipboard::SourceCancelled(wl_data_source* source) noexcept
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] Source cancelled: source=%p",
        static_cast<void*>(source));
    if (source != m_source) {
        return;
    }
    wl_data_source_destroy(m_source);
    m_source = nullptr;
}

void LClipboard::OfferCreated(wl_data_offer* offer) noexcept
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Debug, "[LinuxClipboard] Offer created: offer=%p",
        static_cast<void*>(offer));
    if (m_pendingOffer) {
        wl_data_offer_destroy(m_pendingOffer);
    }
    m_pendingOffer = offer;
    m_offerMimeTypes.clear();
    if (wl_data_offer_add_listener(offer, &DataOfferListener, this) != 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] Failed to register offer listener: offer=%p",
            static_cast<void*>(offer));
        wl_data_offer_destroy(offer);
        m_pendingOffer = nullptr;
        m_offerMimeTypes.clear();
    }
}

void LClipboard::OfferMimeType(wl_data_offer* offer, const char* mimeType) noexcept
{
    if (offer == m_pendingOffer && mimeType) {
        m_offerMimeTypes.emplace_back(mimeType);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Debug,
            "[LinuxClipboard] Offer MIME type: offer=%p type='%s'",
            static_cast<void*>(offer), mimeType);
    }
}

void LClipboard::SelectionChanged(wl_data_offer* offer) noexcept
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Debug,
        "[LinuxClipboard] Selection changed: offer=%p pending=%p current=%p",
        static_cast<void*>(offer), static_cast<void*>(m_pendingOffer),
        static_cast<void*>(m_selectionOffer));
    if (offer == m_selectionOffer) {
        return;
    }
    ResetOffer();
    if (offer == m_pendingOffer) {
        m_selectionOffer = m_pendingOffer;
        m_pendingOffer = nullptr;
    } else if (offer) {
        wl_data_offer_destroy(offer);
    }
}

void LClipboard::TryGetText(TextCallback callback)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClipboard] TryGetText requested: callback=%d pendingFd=%d offer=%p",
        callback != nullptr, m_pendingReadFd, static_cast<void*>(m_selectionOffer));
    if (!callback || m_pendingReadFd >= 0 || !EnsureDevice() || !m_selectionOffer) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] TryGetText rejected");
        return;
    }

    const char* mimeType = nullptr;
    if (std::find(m_offerMimeTypes.begin(), m_offerMimeTypes.end(), Utf8MimeType) != m_offerMimeTypes.end()) {
        mimeType = Utf8MimeType;
    } else if (std::find(m_offerMimeTypes.begin(), m_offerMimeTypes.end(), PlainMimeType) != m_offerMimeTypes.end()) {
        mimeType = PlainMimeType;
    }
    if (!mimeType) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] TryGetText found no supported MIME type");
        CompleteRead(false, {});
        return;
    }

    int pipeDescriptors[2] = {-1, -1};
    if (pipe(pipeDescriptors) != 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info,
            "[LinuxClipboard] TryGetText pipe failed: errno=%d", errno);
        CompleteRead(false, {});
        return;
    }
    const int flags = fcntl(pipeDescriptors[0], F_GETFL, 0);
    fcntl(pipeDescriptors[0], F_SETFL, flags | O_NONBLOCK);
    m_pendingReadFd = pipeDescriptors[0];
    m_pendingReadText.clear();
    m_pendingCallback = std::move(callback);
    wl_data_offer_receive(m_selectionOffer, mimeType, pipeDescriptors[1]);
    close(pipeDescriptors[1]);
    const int flushResult = wl_display_flush(m_windowing.Display());
    const int flushError = errno;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Debug,
        "[LinuxClipboard] Clipboard receive started: mime='%s' fd=%d flush=%d errno=%d",
        mimeType, m_pendingReadFd, flushResult, flushError);
    if (!FlushSucceeded(flushResult, flushError)) {
        ProcessPendingReads();
    }
}

bool LClipboard::Clear()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info,
        "[LinuxClipboard] Clear requested: device=%p serial=%u",
        static_cast<void*>(m_dataDevice), m_windowing.LastInputSerial());
    if (!EnsureDevice() || m_windowing.LastInputSerial() == 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] Clear rejected");
        return false;
    }
    wl_data_device_set_selection(m_dataDevice, nullptr, m_windowing.LastInputSerial());
    if (m_source) {
        wl_data_source_destroy(m_source);
        m_source = nullptr;
    }
    const int flushResult = wl_display_flush(m_windowing.Display());
    const int flushError = errno;
    const bool flushed = FlushSucceeded(flushResult, flushError);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Info, "[LinuxClipboard] Clear %s",
        flushed ? "succeeded" : "failed to flush display");
    return flushed;
}

void LClipboard::ProcessPendingReads() noexcept
{
    if (m_pendingReadFd < 0) {
        return;
    }
    char buffer[4096];
    for (;;) {
        const auto count = read(m_pendingReadFd, buffer, sizeof(buffer));
        if (count > 0) {
            m_pendingReadText.append(buffer, static_cast<std::size_t>(count));
            continue;
        }
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        }
        const bool succeeded = count == 0;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Clipboard,
            NeoWPF::Diagnostics::Level::Debug,
            "[LinuxClipboard] Clipboard read completed: bytes=%zd succeeded=%d errno=%d",
            count, succeeded, count < 0 ? errno : 0);
        const auto text = succeeded ? NeoWPF::Platform::Utf8ToWide(m_pendingReadText) : std::wstring{};
        CompleteRead(succeeded, text);
        return;
    }
}

void LClipboard::CompleteRead(bool succeeded, std::wstring text) noexcept
{
    if (m_pendingReadFd >= 0) {
        close(m_pendingReadFd);
        m_pendingReadFd = -1;
    }
    auto callback = std::move(m_pendingCallback);
    m_pendingReadText.clear();
    auto& appState = NeoWPF::Core::GetAppState();
    if (callback && appState.mainThreadExecutor) {
        appState.mainThreadExecutor->Post([callback = std::move(callback), succeeded, text = std::move(text)]() mutable {
            callback({succeeded, std::move(text)});
        });
    }
}

void LClipboard::ResetOffer() noexcept
{
    if (m_selectionOffer) {
        wl_data_offer_destroy(m_selectionOffer);
    }
    m_selectionOffer = nullptr;
    m_offerMimeTypes.clear();
}

}