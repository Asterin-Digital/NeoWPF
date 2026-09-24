/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Core/ICommand.h>

namespace NeoPM::Core::Daemon {
    class DaemonCommand final : public ICommand {
    public:
        explicit DaemonCommand(bool background = false);
        static bool EnsureRunning();
        static bool StartDetached();

        int Execute() override;
        const char* GetName() const override;
        const char* GetDescription() const override;

    private:
        bool m_background = false;
    };
}