#pragma once

#include <NeoPM/Core/ICommand.h>

namespace NeoPM::Core::Commands {

class RefreshSdkCommand : public ICommand {
public:
    explicit RefreshSdkCommand(const std::string& channel = "");

    int Execute() override;
    const char* GetName() const override;
    const char* GetDescription() const override;

private:
    std::string m_channel;
};

}
