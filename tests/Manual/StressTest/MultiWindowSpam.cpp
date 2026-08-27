// MultiWindowSpam.cpp
// Description: Creates max 25 windows, resizes and closes them every 3 seconds.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <NeoWPF/NeoWPF.h>

int main()
{
    NeoWPF::Diagnostics::PerformanceMetrics::SetEnabled(true);
    NeoWPF::Core::Application::Initialize();

    NeoWPF::Core::Application::Current()->CreateWindow(L"NeoApp", 900, 680);

    using namespace NeoWPF;
    using namespace NeoWPF::UI;

    auto root = std::make_shared<Grid>();
    root->SetBackgroundColor(0xFF0B1220u);
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);

    auto card = std::make_shared<Border>();
    card->SetWidth(520);
    card->SetHeight(160);
    card->SetPadding(16);
    card->SetCornerRadius(CornerRadius(14.0f));
    card->SetBorderThickness(1);
    card->SetBackgroundColor(0xFF0F172Au);
    card->SetBorderColor(0xFF334155u);
    card->SetHorizontalAlignment(HorizontalAlignment::Center);
    card->SetVerticalAlignment(VerticalAlignment::Center);

    auto checkbox = std::make_shared<Checkbox>();
    checkbox->SetText(L"Enable feature");
    checkbox->SetFontSize(16);
    checkbox->SetTextColor(0xFFEDF2F7u);
    checkbox->SetBackgroundColor(0xFF0F172Au);
    checkbox->SetBorderColor(0xFF334155u);
    checkbox->SetPadding(10);
    checkbox->SetHorizontalAlignment(HorizontalAlignment::Center);
    checkbox->SetVerticalAlignment(VerticalAlignment::Center);

    std::atomic<bool> loopActive{false};
    std::atomic<bool> keepRunning{true};
    std::thread worker([&]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> windowCountDist(10, 30);
        std::uniform_int_distribution<> widthDist(200, 600);
        std::uniform_int_distribution<> heightDist(150, 500);
        std::uniform_int_distribution<> delaySectionDist(50, 200);
        std::uniform_int_distribution<> actionDist(0, 1);

        std::vector<std::pair<NeoWPF::Platform::Window, std::chrono::steady_clock::time_point>> windows;
        const int maxWindows = 25;
        const auto windowLifetime = std::chrono::seconds(3);

        while (keepRunning.load()) {
            if (!loopActive.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (!checkbox->IsChecked()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }


            auto now = std::chrono::steady_clock::now();
            for (auto it = windows.begin(); it != windows.end();) {
                if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second) >= windowLifetime) {
                    it->first.Close();
                    it = windows.erase(it);
                } else {
                    ++it;
                }
            }

            if (windows.size() < maxWindows) {
                int windowCount = windowCountDist(gen);
                windowCount = std::min(windowCount, (int)(maxWindows - windows.size()));
                for (int i = 0; i < windowCount; ++i) {
                    auto w = NeoWPF::Core::Application::Current()->CreateWindow(L"NeoWPF Popup", 320, 220);
                    windows.push_back({w, std::chrono::steady_clock::now()});
                }
            }

            int delay = delaySectionDist(gen);
            for (int i = 0; i < delay && keepRunning.load() && loopActive.load(); ++i) {
                if (!windows.empty()) {
                    int action = actionDist(gen);
                    if (action == 0 && !windows.empty()) {

                        int idx = std::uniform_int_distribution<>(0, (int)windows.size() - 1)(gen);
                        int newW = widthDist(gen);
                        int newH = heightDist(gen);
                        windows[idx].first.Resize(newW, newH);

                    } else if (action == 1 && !windows.empty()) {

                        int idx = std::uniform_int_distribution<>(0, (int)windows.size() - 1)(gen);
                        windows[idx].first.Close();
                        windows.erase(windows.begin() + idx);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    });

    checkbox->Checked += [&]() { loopActive.store(true); };
    checkbox->Unchecked += [&]() { loopActive.store(false); };

    auto content = std::make_shared<Grid>();
    content->AddRow(SizeMode::Star, 1);
    content->AddColumn(SizeMode::Star, 1);
    content->AddChild(checkbox, 0, 0);

    card->SetChild(content);
    root->AddChild(card, 0, 0);

    const int result = NeoWPF::Core::Application::Run();

    keepRunning.store(false);
    if (worker.joinable()) {
        worker.join();
    }

    NeoWPF::Core::Application::Shutdown();

    return result;
}

// TEST_METADATA_START
// Test Name: MultiWindowSpam
// Description: Creates max 25 windows, resizes and closes them every 3 seconds.
// Created: 2026-07-16 19:29:11
// Result: Passed
// Notes: No freezes, hangs, crashes or unresponsive states during or after the test. Was still able to interact the app and use it (though with lag)

// [Perf] periodic frames=1180 layouts=133 renders=147315 ticks=1180 creates=1099 avgLayout=0.0477 avgRender=0.0055 avgTick=0.0000 maxFrame=0.0000 maxLayout=0.1976 maxRender=0.2053 maxTick=0.0000 mem=92MB peakMem=92MB cpu=12.4% peakCpu=39.6%
// Path: StressTest/MultiWindowSpam.cpp
// Last Updated: 2026-07-16 19:31:22
// TEST_METADATA_END
