#pragma once

#include <optional>
#include <string>

namespace NeoPM::Utils {

class User {
public:
    static std::optional<std::wstring> GetCurrentUsername();
};

}
