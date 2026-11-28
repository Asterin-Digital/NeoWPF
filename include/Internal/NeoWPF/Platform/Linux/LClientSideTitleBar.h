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

#include <NeoWPF/Contracts/IWindowChrome.h>

#include <memory>
#include <string>

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Platform::Linux {

std::shared_ptr<NeoWPF::UI::Visual> CreateDefaultClientTitleBar(void* nativeHandle,
                                                                const std::wstring& title = L"NeoWPF");
void EnsureClientTitleBar(void* nativeHandle, NeoWPF::Contracts::ChromeMode mode);

}
