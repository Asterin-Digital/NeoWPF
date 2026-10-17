## NeoWPF — A Modern C++ UI Framework

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20Android-lightgrey)](Compile.md)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)](Compile.md)
[![Docs](https://img.shields.io/badge/docs-website-informational)](https://neowpf.github.io/Docs/)
[![Docs Source](https://img.shields.io/badge/docs-source-lightgrey)](https://github.com/NeoWPF/Docs)

### Design Goals

There's a myriad of frameworks out there, and each may have their own reason to exist. NeoWPF has these design goals:

- Familiar : bring the WPF development model to native C++, including XAML, dependency properties, resources, and data binding

- Native : built around C++ from the ground up, rather than wrapping another UI framework

- Cross-platform : support multiple operating systems while keeping application code largely platform independent.

### Platform Support

| Platform | Status | Notes |
|---|---|---|
| Windows | ✅ Most mature | Primary development target |
| Linux   | 🟡 Partial | Half the maturity of Windows |

Half the maturity here means that Linux is less used then Windows, not that it's missing feature's or just doesn't work.

### Quick Start
```cpp
#include <NeoWPF/NeoWPF.h>
#include <memory>

int main()
{
    using namespace NeoWPF::UI;

    NeoWPF::Core::Application::Initialize();

    auto& app = NeoWPF::Core::Application::Current();
    auto window = app.CreateWindow(L"NeoWPF", 800, 600);

    auto root = std::make_shared<Grid>();
    root->SetBackgroundBrush(std::make_shared<SolidColorBrush>(0xFF1E1E1Eu));

    auto panel = std::make_shared<StackPanel>();
    panel->SetSpacing(16);
    panel->SetHorizontalAlignment(HorizontalAlignment::Center);
    panel->SetVerticalAlignment(VerticalAlignment::Center);

    auto title = std::make_shared<TextBlock>(L"Hello, NeoWPF");
    title->SetFontSize(32);
    title->SetForeground(std::make_shared<SolidColorBrush>(0xFFFFFFFFu));
    panel->AddChild(title);

    auto button = std::make_shared<Button>(L"Click me");
    button->SetWidth(200);
    button->SetHeight(45);
    button->SetBackgroundBrush(std::make_shared<SolidColorBrush>(0xFF4F86C6u));
    button->Click += []() {
        // ur click logic
    };
    panel->AddChild(button);

    root->AddChild(panel);
    window.SetContent(root);

    auto result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
```

For XAML and better usage of NeoWPF, NeoPM is required which also gives support for XAML, project management, resources and much more.

### History

NeoWPF started off as one small project because I wanted WPF but made in a native and modern language, like C++.

At first it was small and locked to Windows, but as time went on I kept working more and more on NeoWPF and got it to where it's now.

I started off as only one developer working on the entire project, but as time passed I eventually got a bit of help from my friends and the community as well.

### Vision

I believe NeoWPF has a lot of potential to become widely used across its supported platforms.

With more developers contributing to the project, I see NeoWPF eventually being able to live alongside WPF and Qt without becoming bloated.

### Features

NeoWPF implements the WPF development model natively in C++: XAML, dependency properties, data binding, triggers, and Visual State Manager.

* Visual tree is thread-safe, and custom window chrome is built in

* XAML is compiled at build time by NeoCM rather than parsed at runtime

* NeoPM handles project setup, resources, and builds

* Controls cover Button, TextBox, ComboBox, ListBox, Image, ScrollViewer, and Popup, with Grid, Panel, and Border for layout

* Localization, animation, routed input events, and video and audio playback are included

Full control listings are in UI/Controls and UI/Layout; the remaining subsystems are under UI/, Renderer/, and Platform/.

### Documentation

API index and full documentation: **https://neowpf.github.io/Docs/**

Source files: [NeoWPF/Docs](https://github.com/NeoWPF/Docs).

### License

NeoWPF uses the Apache 2.0 license, See [LICENSE](./LICENSE) for details.
