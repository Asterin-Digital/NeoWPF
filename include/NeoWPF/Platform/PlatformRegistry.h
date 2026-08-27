#pragma once

#include <memory>

#include <NeoWPF/Contracts/IPlatform.h>

namespace NeoWPF::Platform {

void RegisterPlatform(std::unique_ptr<NeoWPF::Contracts::IPlatform> platform);
NeoWPF::Contracts::IPlatform* CurrentPlatform() noexcept;
bool HasPlatform() noexcept;

}
