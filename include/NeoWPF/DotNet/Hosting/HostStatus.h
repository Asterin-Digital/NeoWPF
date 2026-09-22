/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace NeoWPF::DotNet::Hosting {

enum class HostStatusCode : std::uint32_t {
    Ok,
    UnsupportedPlatform,
    RuntimeNotFound,
    HostFxrLoadFailed,
    HostFxrSymbolMissing,
    FrameworkMissing,
    RuntimeInitFailed,
    BridgeMissing,
    BridgeLoadFailed,
    AbiMismatch,
    AlreadyStarted,
    Stopped,
};

class HostStatus {
public:
    HostStatus() = default;
    explicit HostStatus(HostStatusCode code) noexcept : m_code(code) {}

    HostStatusCode Code() const noexcept { return m_code; }
    bool Succeeded() const noexcept { return m_code == HostStatusCode::Ok; }

    const std::string& Message() const noexcept { return m_message; }
    const std::string& Hint() const noexcept { return m_hint; }

    HostStatus& SetMessage(std::string message) {
        m_message = std::move(message);
        return *this;
    }

    HostStatus& SetHint(std::string hint) {
        m_hint = std::move(hint);
        return *this;
    }

private:
    HostStatusCode m_code = HostStatusCode::Ok;
    std::string m_message;
    std::string m_hint;
};

}
