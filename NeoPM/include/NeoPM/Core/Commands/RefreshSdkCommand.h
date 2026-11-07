/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Core/ICommand.h>

namespace NeoPM::Core::Commands {

class RefreshSdkCommand : public ICommand {
public:
    int Execute() override;
    const char* GetName() const override;
    const char* GetDescription() const override;

};

}
