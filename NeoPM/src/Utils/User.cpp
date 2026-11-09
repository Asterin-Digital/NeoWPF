#include <NeoPM/Utils/User.h>

#include <Winux/Winux.h>

namespace NeoPM::Utils {

std::optional<std::wstring> User::GetCurrentUsername()
{
    const auto username = Winux::Get<Winux::System>().GetUsername();
    if (username.Failed() || username.Value().empty()) {
        return std::nullopt;
    }

    return username.Value();
}

}
