#include <NeoWPF/Diagnostics/FailureHooks.h>
#include <atomic>

namespace NeoWPF::Diagnostics {

static std::atomic<FailureHookPoint> s_failureHookPoint{FailureHookPoint::None};

void SetFailureHookPoint(FailureHookPoint point)
{

// prevent devs from running this in Release builds
#if defined(_DEBUG)
    s_failureHookPoint.store(point, std::memory_order_release);
#else
    (void)point;
    s_failureHookPoint.store(FailureHookPoint::None, std::memory_order_release);
#endif
}

FailureHookPoint GetFailureHookPoint()
{
// prevent devs from running this in Release builds
#if defined(_DEBUG)
    return s_failureHookPoint.load(std::memory_order_acquire);
#else
    return FailureHookPoint::None;
#endif
}

}
