#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/Platform/Services/Clipboard.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <algorithm>
#include <memory>

namespace NeoWPF::Diagnostics {
namespace {

bool g_suppressDeveloperWarnings = false;
bool g_isBuildingWarningOverlay = false;
bool g_hasQueuedDeveloperWarning = false;
std::optional<DeveloperWarningMessage> g_lastDeveloperWarning;

class DeveloperWarningOverlay final : public NeoWPF::UI::Grid {
public:
    void SetDialogCard(const std::shared_ptr<NeoWPF::UI::Border>& card)
    {
        m_dialogCard = card;
    }

    void Measure(double availableWidth, double availableHeight) override
    {
        if (auto card = m_dialogCard.lock()) {
            card->SetWidth((std::max)(0.0, (std::min)(760.0, availableWidth - 48.0)));
        }

        NeoWPF::UI::Grid::Measure(availableWidth, availableHeight);
        SetDesiredSize(availableWidth, availableHeight);
    }

private:
    std::weak_ptr<NeoWPF::UI::Border> m_dialogCard;
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

std::shared_ptr<NeoWPF::UI::Popup> CreateDeveloperWarningPopup(
    const DeveloperWarningMessage& warning,
    NeoWPF::UI::FrameworkElement* placementTarget)
{
    const auto accentColor = warning.severity == DeveloperWarningSeverity::Error
        ? 0xFFF87171u
        : 0xFFFBBF24u;
    const auto severityText = warning.severity == DeveloperWarningSeverity::Error
        ? L"DEVELOPER ERROR"
        : L"DEVELOPER WARNING";

    auto popup = std::make_shared<NeoWPF::UI::Popup>();
    popup->SetPlacementTarget(placementTarget);
    popup->SetPlacement(NeoWPF::UI::PopupPlacement::Center);
    popup->SetStaysOpen(true);

    auto overlay = std::make_shared<DeveloperWarningOverlay>();
    overlay->AddRow(NeoWPF::UI::SizeMode::Star, 1);
    overlay->AddColumn(NeoWPF::UI::SizeMode::Star, 1);

    auto backdrop = std::make_shared<NeoWPF::UI::Border>();
    backdrop->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xB8000000u));
    overlay->AddChild(backdrop);

    auto card = std::make_shared<NeoWPF::UI::Border>();
    card->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    card->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    card->SetPadding(28);
    card->SetBorderThickness(1);
    card->SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF3A4655u));
    card->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF202A36u));
    card->SetCornerRadius(NeoWPF::UI::CornerRadius(16));

    auto content = std::make_shared<NeoWPF::UI::StackPanel>();
    content->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);

    auto severity = std::make_shared<NeoWPF::UI::TextBlock>(severityText);
    severity->SetFontSize(12);
    severity->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(accentColor));
    severity->SetMargin(0, 0, 0, 8);

    auto title = std::make_shared<NeoWPF::UI::TextBlock>(warning.title);
    title->SetFontSize(25);
    title->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFF8FAFCu));
    title->SetMargin(0, 0, 0, 22);

    auto details = std::make_shared<NeoWPF::UI::Border>();
    details->SetPadding(20);
    details->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF151D26u));
    details->SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF303C49u));
    details->SetBorderThickness(1);
    details->SetCornerRadius(NeoWPF::UI::CornerRadius(10));

    auto message = std::make_shared<NeoWPF::UI::TextBlock>(warning.message);
    message->SetFontSize(15);
    message->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFD6DEE8u));
    message->SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    details->SetContent(message);

    auto actions = std::make_shared<NeoWPF::UI::StackPanel>();
    actions->SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    actions->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Right);
    actions->SetMargin(0, 24, 0, 0);

    auto copy = std::make_shared<NeoWPF::UI::Button>(L"Copy");
    copy->SetWidth(112);
    copy->SetHeight(42);
    copy->SetMargin(0, 0, 10, 0);
    copy->SetPadding(12);
    copy->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF2563EBu));
    copy->SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF2563EBu));
    copy->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFFFFFFFu));

    auto close = std::make_shared<NeoWPF::UI::Button>(L"Close");
    close->SetWidth(112);
    close->SetHeight(42);
    close->SetPadding(12);
    close->SetBackgroundBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF303C49u));
    close->SetBorderBrush(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF465365u));
    close->SetForeground(std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFF8FAFCu));

    const std::wstring copyText = warning.title + L"\n\n" + warning.message;
    std::weak_ptr<NeoWPF::UI::Button> weakCopy = copy;
    copy->Click += [copyText, weakCopy]() {
        if (auto button = weakCopy.lock()) {
            const auto result = NeoWPF::Platform::Services::Clipboard().SetText(copyText);
            button->SetText(result.Success() ? L"Copied" : L"Copy failed");
        }
    };

    std::weak_ptr<NeoWPF::UI::Popup> weakPopup = popup;
    close->Click += [weakPopup]() {
        if (auto openPopup = weakPopup.lock()) {
            openPopup->SetIsOpen(false);
        }
    };

    actions->AddChild(copy);
    actions->AddChild(close);
    content->AddChild(severity);
    content->AddChild(title);
    content->AddChild(details);
    content->AddChild(actions);
    card->SetContent(content);
    overlay->AddChild(card);
    overlay->SetDialogCard(card);
    popup->SetContent(overlay);
    return popup;
}

void AttachDeveloperWarningPopup(const DeveloperWarningMessage& warning)
{
    auto& appState = NeoWPF::Core::GetAppState();
    std::shared_ptr<NeoWPF::UI::Window> targetWindow;
    NeoWPF::UI::FrameworkElement* placementTarget = nullptr;

    for (const auto& windowSession : appState.windowSessions) {
        if (windowSession.windowHost && windowSession.windowHost->GetContent()) {
            targetWindow = windowSession.windowHost;
            placementTarget = dynamic_cast<NeoWPF::UI::FrameworkElement*>(
                windowSession.windowHost->GetContent().get());
            if (!placementTarget) {
                placementTarget = windowSession.windowHost.get();
            }
            break;
        }
    }

    if (!targetWindow) {
        return;
    }

    if (DeveloperWarnings::s_debugLogging) {
        NeoWPF::Diagnostics::LogWarn("DeveloperWarnings.cpp: open popup title=%ls message=%ls", warning.title.c_str(), warning.message.c_str());
    }

    WarningOverlayBuildGuard guard(g_isBuildingWarningOverlay);
    auto popup = CreateDeveloperWarningPopup(warning, placementTarget);
    appState.developerWarningOverlayHost = popup;
    popup->SetIsOpen(true);
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
    if (auto popup = std::dynamic_pointer_cast<NeoWPF::UI::Popup>(appState.developerWarningOverlayHost)) {
        popup->SetIsOpen(false);
    }
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

    for (const auto& warning : appState.pendingDeveloperMessages) {
        AttachDeveloperWarningPopup(warning);
    }

    appState.pendingDeveloperMessages.clear();
    g_hasQueuedDeveloperWarning = false;
}

}