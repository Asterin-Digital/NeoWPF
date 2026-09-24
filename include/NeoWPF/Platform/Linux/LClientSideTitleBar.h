/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
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
