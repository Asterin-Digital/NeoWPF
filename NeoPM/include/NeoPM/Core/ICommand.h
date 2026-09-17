#pragma once

#include <string>
#include <memory>

namespace NeoPM::Core {
    class ICommand {
    public:
        virtual ~ICommand() = default;

        virtual int Execute() = 0;
        virtual const char* GetName() const = 0;
        virtual const char* GetDescription() const = 0;
    };
}
