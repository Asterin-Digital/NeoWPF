#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <algorithm>

namespace NeoWPF::UI::Animation {

Timeline::Timeline(float from, float to, float durationSeconds, ValueChangedCallback callback)
    : m_from(from), m_to(to), m_durationSeconds(durationSeconds), m_callback(std::move(callback)) {}

bool Timeline::Update(float elapsedSeconds) {
    if (m_durationSeconds <= 0.0f) {
        m_currentValue = m_to;
        if (m_callback) {
            NeoWPF::Diagnostics::LogInfo("[Animation] timeline completed immediately value=%.3f", m_currentValue);
            m_callback(m_currentValue);
        }
        return true;
    }
    m_elapsedSeconds += elapsedSeconds;
    const float t = std::clamp(m_elapsedSeconds / m_durationSeconds, 0.0f, 1.0f);
    const float eased = t * t * (3.0f - 2.0f * t);
    m_currentValue = m_from + (m_to - m_from) * eased;
    if (m_callback) {
        NeoWPF::Diagnostics::LogInfo("[Animation] timeline progress=%.3f value=%.3f", t, m_currentValue);
        m_callback(m_currentValue);
    }
    return t >= 1.0f;
}

}
