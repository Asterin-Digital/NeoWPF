/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>

#include <NeoWPF/Contracts/IPlatform.h>

namespace NeoWPF::Platform {

void RegisterPlatform(std::unique_ptr<NeoWPF::Contracts::IPlatform> platform);
std::unique_ptr<NeoWPF::Contracts::IPlatform> CreateDefaultPlatform();
NeoWPF::Contracts::IPlatform* CurrentPlatform() noexcept;
bool HasPlatform() noexcept;
[[noreturn]] void ExitForMissingPlatform();
[[noreturn]] void ExitForUninitializedApplication();

}
