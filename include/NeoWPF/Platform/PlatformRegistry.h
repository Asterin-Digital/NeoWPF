#pragma once

#include <memory>

#include <NeoWPF/Contracts/IPlatform.h>

namespace NeoWPF::Platform {

void RegisterPlatform(std::unique_ptr<NeoWPF::Contracts::IPlatform> platform);
std::unique_ptr<NeoWPF::Contracts::IPlatform> CreateDefaultPlatform();
NeoWPF::Contracts::IPlatform* CurrentPlatform() noexcept;
bool HasPlatform() noexcept;

}
