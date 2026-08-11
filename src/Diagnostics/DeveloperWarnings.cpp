#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <memory>

namespace NeoWPF::Diagnostics {
namespace {

bool g_suppressDeveloperWarnings = false;
bool g_isBuildingWarningOverlay = false;
bool g_hasQueuedDeveloperWarning = false;
std::optional<DeveloperWarningMessage> g_lastDeveloperWarning;

struct OverlayBuildResult {
    std::shared_ptr<NeoWPF::UI::Visual> panel;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> ownedVisuals;
};

struct WarningOverlayBuildGuard {
    explicit WarningOverlayBuildGuard(bool& flag)
        : flag(flag)
    {
        flag = true;
    }

    ~WarningOverlayBuildGuard()
    {
        flag = false;
    }

private:
    bool& flag;
};

OverlayBuildResult CreateDeveloperWarningOverlay(const DeveloperWarningMessage& warning, int overlayIndex)
{
    auto card = std::make_shared<NeoWPF::UI::Border>();
    card->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
    card->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);
    card->SetMargin(0, 0, 0, 10);
    card->SetPadding(14);
    card->SetBorderThickness(1);
    card->SetBorderColor(warning.severity == DeveloperWarningSeverity::Error ? 0xFFB00020u : 0xFFB8731Fu);
    card->SetBackgroundColor(warning.severity == DeveloperWarningSeverity::Error ? 0xFF2A1113u : 0xFF2B2418u);
    card->SetCornerRadius(NeoWPF::UI::CornerRadius(12));
    card->SetOpacity(0.98f);
    card->SetZIndex(100000 + overlayIndex);

    auto content = std::make_shared<NeoWPF::UI::StackPanel>();
    content->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);
    content->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);

    auto titleText = std::make_shared<NeoWPF::UI::TextBlock>(warning.title);
    titleText->SetFontSize(14);
    titleText->SetTextColor(0xFFFFFFFFu);
    titleText->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
    titleText->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);

    auto bodyText = std::make_shared<NeoWPF::UI::TextBlock>(warning.message);
    bodyText->SetFontSize(12);
    bodyText->SetTextColor(0xFFEDEDEDu);
    bodyText->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
    bodyText->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);

    content->AddVisualChild(titleText);
    content->AddVisualChild(bodyText);
    card->SetContent(content);

    return {card, {card, content, titleText, bodyText}};
}

void AttachDeveloperWarningOverlayToRoot(const DeveloperWarningMessage& warning, int overlayIndex)
{
    auto& appState = NeoWPF::Core::GetAppState();
    NeoWPF::UI::Visual* targetRoot = nullptr;

    for (const auto& windowState : appState.windows) {
        if (windowState.windowHost && windowState.windowHost->GetContent()) {
            targetRoot = windowState.windowHost->GetContent().get();
            break;
        }
    }

    if (!targetRoot) {
        return;
    }

    if (DeveloperWarnings::s_debugLogging) {
        NeoWPF::Diagnostics::LogWarn("DeveloperWarnings.cpp: attach overlay index=%d title=%ls message=%ls", overlayIndex, warning.title.c_str(), warning.message.c_str());
    }

    WarningOverlayBuildGuard guard(g_isBuildingWarningOverlay);
    auto overlay = CreateDeveloperWarningOverlay(warning, overlayIndex);

    if (!appState.developerWarningOverlayHost) {
        auto overlayHost = std::make_shared<NeoWPF::UI::StackPanel>();
        overlayHost->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Right);
        overlayHost->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);
        overlayHost->SetMargin(16, 16, 16, 16);
        overlayHost->SetZIndex(100000);
        targetRoot->AddVisualChild(overlayHost);
        appState.developerWarningOverlayHost = overlayHost;
        appState.developerOverlayVisuals.push_back(overlayHost);
    }

    auto overlayHost = std::dynamic_pointer_cast<NeoWPF::UI::StackPanel>(appState.developerWarningOverlayHost);
    if (overlayHost) {
        overlayHost->AddVisualChild(overlay.panel);
    }

    for (const auto& visual : overlay.ownedVisuals) {
        appState.developerOverlayVisuals.push_back(visual);
    }
}

}

bool DeveloperWarnings::s_debugLogging = true;

bool IsDeveloperWarningSuppressionEnabled() noexcept
{
    return g_suppressDeveloperWarnings;
}

void SetDeveloperWarningSuppression(bool enabled) noexcept
{
    g_suppressDeveloperWarnings = enabled;
}

void QueueDeveloperWarning(const std::wstring& title, const std::wstring& message, DeveloperWarningSeverity severity)
{
    if (g_suppressDeveloperWarnings || g_isBuildingWarningOverlay || g_hasQueuedDeveloperWarning) {
        return;
    }

    if (DeveloperWarnings::s_debugLogging) {
        NeoWPF::Diagnostics::LogWarn("DeveloperWarnings.cpp: queue title=%ls severity=%d message=%ls", title.c_str(), static_cast<int>(severity), message.c_str());
    }

    g_hasQueuedDeveloperWarning = true;
    g_lastDeveloperWarning = DeveloperWarningMessage{title, message, severity};
    auto& appState = NeoWPF::Core::GetAppState();
    appState.pendingDeveloperMessages.push_back({title, message, severity});
}

std::optional<DeveloperWarningMessage> GetLastWarning() noexcept
{
    return g_lastDeveloperWarning;
}

void ClearDeveloperWarnings()
{
    g_hasQueuedDeveloperWarning = false;
    auto& appState = NeoWPF::Core::GetAppState();
    appState.pendingDeveloperMessages.clear();
    appState.developerOverlayVisuals.clear();
    appState.developerWarningOverlayHost.reset();
}

void ProcessPendingDeveloperWarnings()
{
    auto& appState = NeoWPF::Core::GetAppState();

    if (appState.pendingDeveloperMessages.empty()) {
        return;
    }

    if (DeveloperWarnings::s_debugLogging) {
        NeoWPF::Diagnostics::LogWarn("DeveloperWarnings.cpp: flush count=%zu", appState.pendingDeveloperMessages.size());
    }

    for (size_t index = 0; index < appState.pendingDeveloperMessages.size(); ++index) {
        AttachDeveloperWarningOverlayToRoot(appState.pendingDeveloperMessages[index], static_cast<int>(index));
    }

    appState.pendingDeveloperMessages.clear();
    g_hasQueuedDeveloperWarning = false;
}

}