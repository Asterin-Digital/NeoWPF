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

#include <NeoWPF/DotNet/Hosting/HostOptions.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path ResolvePath(const HostOptions& options);
HostStatus ResolveStatus(const HostOptions& options);

}
