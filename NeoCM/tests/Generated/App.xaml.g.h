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
