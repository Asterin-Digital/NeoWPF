#include <gtest/gtest.h>

#include <NeoWPF/Platform/Linux/LCompositor.h>

namespace {

TEST(LinuxBackendTests, OverrideSelectsExplicitBackend)
{
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend("wayland", "x11", "wayland0"),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::Wayland);
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend("x11", "wayland", nullptr),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::X11);
}

TEST(LinuxBackendTests, SessionTypeTakesPrecedenceOverDisplayVariables)
{
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend(nullptr, "x11", "wayland0"),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::X11);
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend(nullptr, "wayland", "wayland0"),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::Wayland);
}

TEST(LinuxBackendTests, WaylandDisplayEnablesWaylandDetection)
{
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend(nullptr, nullptr, "wayland0"),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::Wayland);
}

TEST(LinuxBackendTests, X11IsTheDefaultWhenNoDisplayProtocolIsSet)
{
    EXPECT_EQ(
        NeoWPF::Platform::Linux::ResolveLinuxDisplayBackend(nullptr, nullptr, nullptr),
        NeoWPF::Platform::Linux::LinuxDisplayBackend::X11);
}

}
