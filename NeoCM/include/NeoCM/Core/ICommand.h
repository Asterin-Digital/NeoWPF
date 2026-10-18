/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

namespace NeoCM::Core {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual int Execute() = 0;
};

}
