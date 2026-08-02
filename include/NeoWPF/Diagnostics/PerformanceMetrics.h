#pragma once

#include <cstdint>

namespace NeoWPF::Diagnostics::PerformanceMetrics {

void Initialize();
void Shutdown();
void SetEnabled(bool enabled);
bool IsEnabled();
void RecordStartup();
void RecordFrameSample(double elapsedSeconds);
void RecordLayoutPass(double elapsedSeconds);
void RecordRenderPass(double elapsedSeconds);
void RecordMessageLoopTick(double elapsedSeconds);
void RecordWindowCreate();
void RecordWindowClose();
void SampleSystemMetrics();
void EmitPeriodicSummary();
void ForceSummary();

}
