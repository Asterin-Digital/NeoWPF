#include <NeoWPF/Platform/Android/AWindow.h>

namespace NeoWPF::Platform::Android {

AWindow::~AWindow()
{
    SetNativeWindow(nullptr);
}

ANativeWindow* AWindow::NativeWindow() const noexcept
{
    return m_nativeWindow.load(std::memory_order_acquire);
}

void AWindow::SetNativeWindow(ANativeWindow* nativeWindow) noexcept
{
    ANativeWindow* previous = m_nativeWindow.exchange(nativeWindow, std::memory_order_acq_rel);
    if (previous) {
        ANativeWindow_release(previous);
    }
}

void AWindow::SetSize(unsigned int width, unsigned int height) noexcept
{
    const auto size = (static_cast<std::uint64_t>(width) << 32) | height;
    m_size.store(size, std::memory_order_release);
}

void AWindow::GetSize(unsigned int& width, unsigned int& height) const noexcept
{
    const auto size = m_size.load(std::memory_order_acquire);
    width = static_cast<unsigned int>(size >> 32);
    height = static_cast<unsigned int>(size);
}

}