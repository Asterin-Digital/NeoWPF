/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
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

    /*
        @summary
        Gets the raw hostfxr result associated with this status.
    */
    std::int32_t GetRawHostFxrResult() const noexcept { return m_rawHostFxrResult; }

    const std::string& Message() const noexcept { return m_message; }
    const std::string& Hint() const noexcept { return m_hint; }

    /*
        @summary
        Sets the raw hostfxr result associated with this status.

        @param result
        Result returned by a hostfxr operation, or zero when unavailable.
    */
    HostStatus& SetRawHostFxrResult(std::int32_t result) noexcept {
        m_rawHostFxrResult = result;
        return *this;
    }

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
    std::int32_t m_rawHostFxrResult = 0;
    std::string m_message;
    std::string m_hint;
};

}
