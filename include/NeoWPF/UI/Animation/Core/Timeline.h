#pragma once

#include <functional>

namespace NeoWPF::UI::Animation {

class Timeline {
public:
    using ValueChangedCallback = std::function<void(float)>;

    Timeline(float from, float to, float durationSeconds, ValueChangedCallback callback);
    bool Update(float elapsedSeconds);

private:
    float m_from = 0.0f;
    float m_to = 0.0f;
    float m_durationSeconds = 0.0f;
    float m_currentValue = 0.0f;
    float m_elapsedSeconds = 0.0f;
    ValueChangedCallback m_callback;
};

}
