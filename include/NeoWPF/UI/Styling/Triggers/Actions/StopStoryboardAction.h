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

#include <NeoWPF/UI/Styling/Triggers/Actions/TriggerAction.h>

namespace NeoWPF::UI::Styling::Triggers {

class StopStoryboardAction : public TriggerAction {
public:
    explicit StopStoryboardAction(std::uint64_t animationHandle = 0);
    void Invoke(UIElement* element) const override;

private:
    std::uint64_t m_animationHandle;
};

}
