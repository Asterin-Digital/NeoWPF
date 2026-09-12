#pragma once

namespace NeoCM::Core {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual int Execute() = 0;
};

}
