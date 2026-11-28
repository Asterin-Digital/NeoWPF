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

#include <NeoWPF/UI/Styling/Triggers/Actions/TriggerAction.h>

namespace NeoWPF::UI::Animation { class Storyboard; }

namespace NeoWPF::UI::Styling::Triggers {

class BeginStoryboardAction : public TriggerAction {
public:
    explicit BeginStoryboardAction(std::shared_ptr<UI::Animation::Storyboard> storyboard);
    void Invoke(UIElement* element) const override;

private:
    std::shared_ptr<UI::Animation::Storyboard> m_storyboard;
};

}
