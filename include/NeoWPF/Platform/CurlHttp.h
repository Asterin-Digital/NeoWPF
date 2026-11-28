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

#include <cstdint>
#include <string>
#include <vector>

#include <NeoWPF/Feedback/Result.h>

namespace NeoWPF::Platform {

class CurlHttp final {
public:
    NeoWPF::Result<std::vector<std::uint8_t>> Fetch(const std::wstring& url) const;
};

}