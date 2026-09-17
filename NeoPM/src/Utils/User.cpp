#include <NeoPM/Utils/User.h>

#include <Winux/Winux.h>

namespace NeoPM::Utils {

std::optional<std::wstring> User::GetCurrentUsername()
{
    const auto platform = Winux::Platform::create();
    if (!platform) {
        return std::nullopt;
    }

    const auto username = platform->system().get_username();
    if (username.failed() || username.value().empty()) {
        return std::nullopt;
    }

    return username.value();
}

}
