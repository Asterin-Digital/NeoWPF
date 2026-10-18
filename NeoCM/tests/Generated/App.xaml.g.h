/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/Core/Application/Application.h>

namespace NeoCM::Tests {

class App : public NeoWPF::Core::Application {
public:
    App();

    void InitializeComponent();
};

}
