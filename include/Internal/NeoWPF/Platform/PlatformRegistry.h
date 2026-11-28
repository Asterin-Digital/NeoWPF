/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
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