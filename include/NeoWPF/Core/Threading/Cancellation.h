/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <utility>

namespace NeoWPF::Core::Threading {

class CancellationToken {
public:
    CancellationToken() noexcept = default;

    bool IsCancellationRequested() const noexcept;

private:
    struct State;
    explicit CancellationToken(std::shared_ptr<State> state) noexcept;

    std::shared_ptr<State> m_state;
    friend class CancellationSource;
};

class CancellationSource {
public:
    CancellationSource();

    CancellationToken GetToken() const noexcept;
    bool IsCancellationRequested() const noexcept;
    bool RequestCancellation() noexcept;

private:
    std::shared_ptr<CancellationToken::State> m_state;
};

}
