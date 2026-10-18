/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
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
