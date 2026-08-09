#include <NeoWPF/NeoWPF.h>
#include <memory>
#include <thread>
#include <chrono>

int main()
{
    using namespace NeoWPF;
    using namespace Diagnostics;

    /*SetGlobalLevel(Level::Trace);
    SetLevel(Category::Visual, Level::Off);
    SetLevel(Category::GridLayout, Level::Debug);
    SetLevel(Category::ContentPresenter, Level::Off);
    SetLevel(Category::Control, Level::Debug);
    SetLevel(Category::Application, Level::Off);
    SetLevel(Category::TextBox, Level::Off);
    SetLevel(Category::HitTest, Level::Off);*/

    Core::Application::Initialize();

    auto window = Core::Application::Current()->CreateWindow(L"Saphire Chat", 980, 640);

    auto rootGrid = std::make_shared<UI::Grid>();
    rootGrid->AddRow(UI::SizeMode::Star, 1);
    rootGrid->AddColumn(UI::SizeMode::Auto);
    rootGrid->AddColumn(UI::SizeMode::Star, 1);
    rootGrid->SetBackgroundColor(0xFF0F172A);

    auto sidebar = std::make_shared<UI::Border>();
    sidebar->SetBackgroundColor(0xFF111827);
    sidebar->SetBorderColor(0xFF243244u);
    sidebar->SetBorderThickness(1);
    sidebar->SetWidth(280);
    sidebar->SetPadding(16);

    auto sidebarStack = std::make_shared<UI::StackPanel>();
    sidebarStack->SetOrientation(UI::Orientation::Vertical);
    sidebarStack->SetSpacing(18);

    auto sidebarTitle = std::make_shared<UI::TextBlock>();
    sidebarTitle->SetText(L"Saphire Discord");
    sidebarTitle->SetFontSize(22);
    sidebarTitle->SetTextColor(0xFFFFFFFFu);

    auto sidebarSubtitle = std::make_shared<UI::TextBlock>();
    sidebarSubtitle->SetText(L"Modern Chat Interface");
    sidebarSubtitle->SetFontSize(12);
    sidebarSubtitle->SetTextColor(0xFF94A3B8u);

    auto channelHeader = std::make_shared<UI::TextBlock>();
    channelHeader->SetText(L"Channels");
    channelHeader->SetFontSize(13);
    channelHeader->SetTextColor(0xFF94A3B8u);

    auto createChannelButton = std::make_shared<UI::Button>(L"+ New Channel");
    createChannelButton->SetPadding(10);
    createChannelButton->SetBackgroundColor(0xFF1E293B);
    createChannelButton->SetBorderColor(0xFF334155u);
    createChannelButton->SetTextColor(0xFFE2E8F0u);
    createChannelButton->SetWidth(248);

    auto channelList = std::make_shared<UI::StackPanel>();
    channelList->SetOrientation(UI::Orientation::Vertical);
    channelList->SetSpacing(8);

    auto makeChannelItem = [&](const std::wstring& text, bool active) {
        auto itemBorder = std::make_shared<UI::Border>();
        itemBorder->SetBackgroundColor(active ? 0xFF1D4ED8u : 0xFF0F172A);
        itemBorder->SetBorderColor(active ? 0xFF60A5FAu : 0x00000000u);
        itemBorder->SetBorderThickness(active ? 1 : 0);
        itemBorder->SetPadding(10);
        itemBorder->SetCornerRadius(UI::CornerRadius(10));

        auto itemText = std::make_shared<UI::TextBlock>();
        itemText->SetText(text);
        itemText->SetFontSize(14);
        itemText->SetTextColor(active ? 0xFFFFFFFFu : 0xFFCBD5E1u);

        itemBorder->SetChild(itemText);
        return itemBorder;
    };

    channelList->AddChild(makeChannelItem(L"# general", true));
    channelList->AddChild(makeChannelItem(L"# design", false));
    channelList->AddChild(makeChannelItem(L"# development", false));
    channelList->AddChild(makeChannelItem(L"# streaming", false));

    auto friendsHeader = std::make_shared<UI::TextBlock>();
    friendsHeader->SetText(L"Friends");
    friendsHeader->SetFontSize(13);
    friendsHeader->SetTextColor(0xFF94A3B8u);

    auto friendList = std::make_shared<UI::StackPanel>();
    friendList->SetOrientation(UI::Orientation::Vertical);
    friendList->SetSpacing(8);

    auto makeFriendItem = [&](const std::wstring& name, const std::wstring& status) {
        auto friendBorder = std::make_shared<UI::Border>();
        friendBorder->SetBackgroundColor(0xFF111827);
        friendBorder->SetBorderColor(0xFF1F2937u);
        friendBorder->SetBorderThickness(1);
        friendBorder->SetPadding(10);
        friendBorder->SetCornerRadius(UI::CornerRadius(10));

        auto friendStack = std::make_shared<UI::StackPanel>();
        friendStack->SetOrientation(UI::Orientation::Vertical);
        friendStack->SetSpacing(3);

        auto friendName = std::make_shared<UI::TextBlock>();
        friendName->SetText(name);
        friendName->SetFontSize(13);
        friendName->SetTextColor(0xFFFFFFFFu);

        auto friendStatus = std::make_shared<UI::TextBlock>();
        friendStatus->SetText(status);
        friendStatus->SetFontSize(11);
        friendStatus->SetTextColor(0xFF94A3B8u);

        friendStack->AddChild(friendName);
        friendStack->AddChild(friendStatus);
        friendBorder->SetChild(friendStack);
        return friendBorder;
    };

    friendList->AddChild(makeFriendItem(L"Mira", L"Online"));
    friendList->AddChild(makeFriendItem(L"Aria", L"Do Not Disturb"));
    friendList->AddChild(makeFriendItem(L"Leo", L"Idle"));

    sidebarStack->AddChild(sidebarTitle);
    sidebarStack->AddChild(sidebarSubtitle);
    sidebarStack->AddChild(channelHeader);
    sidebarStack->AddChild(createChannelButton);
    sidebarStack->AddChild(channelList);
    sidebarStack->AddChild(friendsHeader);
    sidebarStack->AddChild(friendList);
    sidebar->SetChild(sidebarStack);

    auto mainGrid = std::make_shared<UI::Grid>();
    mainGrid->AddRow(UI::SizeMode::Auto);
    mainGrid->AddRow(UI::SizeMode::Star, 1);
    mainGrid->AddRow(UI::SizeMode::Auto);
    mainGrid->SetBackgroundColor(0xFF111827);
    mainGrid->SetMargin(UI::Thickness(20, 16, 16, 16));

    auto headerBar = std::make_shared<UI::Border>();
    headerBar->SetBackgroundColor(0xFF0F172A);
    headerBar->SetBorderColor(0xFF263549u);
    headerBar->SetBorderThickness(1);
    headerBar->SetPadding(16);
    headerBar->SetCornerRadius(UI::CornerRadius(14));

    auto headerText = std::make_shared<UI::TextBlock>();
    headerText->SetText(L"# general — Saphire Chat Room");
    headerText->SetFontSize(18);
    headerText->SetTextColor(0xFFFFFFFFu);
    headerBar->SetChild(headerText);

    auto messageAreaBorder = std::make_shared<UI::Border>();
    messageAreaBorder->SetBackgroundColor(0xFF0C1320);
    messageAreaBorder->SetBorderColor(0xFF263549u);
    messageAreaBorder->SetBorderThickness(1);
    messageAreaBorder->SetPadding(10);
    messageAreaBorder->SetCornerRadius(UI::CornerRadius(14));

    auto messageScroll = std::make_shared<UI::ScrollViewer>();
    messageScroll->SetHorizontalScrollBarVisibility(UI::ScrollBarVisibility::Disabled);
    messageScroll->SetVerticalScrollBarVisibility(UI::ScrollBarVisibility::Auto);

    auto messages = std::make_shared<UI::ItemsControl>();
    messages->SetPadding(8);
    messages->SetVerticalSpacing(12);

    auto makeMessageBubble = [&](const std::wstring& author,
                                 const std::wstring& body,
                                 const std::wstring& time,
                                 bool mine) {
        auto messageGrid = std::make_shared<UI::Grid>();
        messageGrid->AddColumn(UI::SizeMode::Star, 1);
        messageGrid->AddColumn(UI::SizeMode::Auto);
        messageGrid->SetHorizontalSpacing(10);

        auto bubble = std::make_shared<UI::Border>();
        bubble->SetPadding(12);
        bubble->SetBorderThickness(1);
        bubble->SetBorderColor(mine ? 0xFF3B82F6u : 0xFF2A3748u);
        bubble->SetBackgroundColor(mine ? 0xFF2563EBu : 0xFF1F2937u);
        bubble->SetCornerRadius(UI::CornerRadius(16));

        auto bubbleStack = std::make_shared<UI::StackPanel>();
        bubbleStack->SetOrientation(UI::Orientation::Vertical);
        bubbleStack->SetSpacing(5);

        auto authorText = std::make_shared<UI::TextBlock>();
        authorText->SetText(author + L"  •  " + time);
        authorText->SetFontSize(12);
        authorText->SetTextColor(mine ? 0xFFE0F2FEu : 0xFF94A3B8u);

        auto bodyText = std::make_shared<UI::TextBlock>();
        bodyText->SetText(body);
        bodyText->SetFontSize(14);
        bodyText->SetTextColor(0xFFE2E8F0u);

        bubbleStack->AddChild(authorText);
        bubbleStack->AddChild(bodyText);
        bubble->SetChild(bubbleStack);

        if (mine) {
            messageGrid->AddChild(bubble, 0, 1);
        }
        else {
            messageGrid->AddChild(bubble, 0, 0);
        }

        return messageGrid;
    };

    const std::vector<std::tuple<std::wstring, std::wstring, std::wstring, bool>> sampleMessages = {
        {L"Nova", L"Hey team, I just pushed the latest layout changes for the dark theme.", L"9:18 AM", false},
        {L"You", L"Looks great! The sidebar and message style feel much cleaner now.", L"9:19 AM", true},
        {L"Mira", L"I think we should also add a quick action button for attachments.", L"9:21 AM", false},
        {L"You", L"Yes, I can add an icon button near the message input.", L"9:22 AM", true},
        {L"Aria", L"The new cards are reading much better with the higher contrast. Nice work.", L"9:24 AM", false},
        {L"You", L"Thanks. I’m tightening the spacing and accent tones so the interface feels more premium.", L"9:25 AM", true},
        {L"Leo", L"Need to make sure the message list still remains readable in the longer threads.", L"9:27 AM", false},
        {L"You", L"I’ve already adjusted the bubble padding and text contrast to improve readability.", L"9:28 AM", true},
        {L"Nova", L"The new scroll area feels much nicer too. We can keep the chat dense without sacrificing clarity.", L"9:29 AM", false},
        {L"Mira", L"I’d love to see a subtle hover state for channels and maybe a richer accent color in the active item.", L"9:31 AM", false},
        {L"You", L"That can be added without changing the dark theme. I’ll keep it subtle and polished.", L"9:33 AM", true},
        {L"Aria", L"I’m also thinking about a lower-noise sidebar so the channel names and status feel a little more premium.", L"9:35 AM", false},
        {L"Nova", L"We should keep the content balanced and not overdo the saturation. The layout is already feeling more modern.", L"9:37 AM", false},
        {L"You", L"Agreed. This pass is meant to be comfortable, crisp, and readable at a glance.", L"9:38 AM", true},
        {L"Leo", L"Perfect. I’ll start polishing the remaining utilities next. The new layout feels much more intentional.", L"9:40 AM", false},
        {L"You", L"Great. Let’s keep iterating and make sure the channel list and activity panel both hold up with longer threads.", L"9:42 AM", true},
        {L"Mira", L"I’m on board. The contrast is finally strong enough that the text feels deliberate rather than flattened.", L"9:44 AM", false},
        {L"Nova", L"This is a good balance. The chat now feels like a real product mockup instead of a raw prototype.", L"9:46 AM", false}
    };

    for (const auto& msg : sampleMessages) {
        const auto& [author, body, time, mine] = msg;
        messages->AddItem(makeMessageBubble(author, body, time, mine));
    }

    messageScroll->SetContent(messages);
    messageAreaBorder->SetChild(messageScroll);

    auto inputBorder = std::make_shared<UI::Border>();
    inputBorder->SetBackgroundColor(0xFF0F172A);
    inputBorder->SetBorderColor(0xFF263549u);
    inputBorder->SetBorderThickness(1);
    inputBorder->SetPadding(12);
    inputBorder->SetCornerRadius(UI::CornerRadius(14));

    auto inputRow = std::make_shared<UI::Grid>();
    inputRow->AddColumn(UI::SizeMode::Star, 1);
    inputRow->AddColumn(UI::SizeMode::Auto);

    auto chatInput = std::make_shared<UI::TextBox>();
    chatInput->SetText(L"Write a message...");
    chatInput->SetPadding(12);
    chatInput->SetBackgroundColor(0xFF111827u);
    chatInput->SetBorderColor(0xFF334155u);
    chatInput->SetTextColor(0xFFFFFFFFu);
    chatInput->SetHorizontalAlignment(UI::HorizontalAlignment::Stretch);

    auto sendButton = std::make_shared<UI::Button>(L"Send");
    sendButton->SetPadding(12);
    sendButton->SetBackgroundColor(0xFF2563EBu);
    sendButton->SetBorderColor(0xFF3B82F6u);
    sendButton->SetTextColor(0xFFFFFFFFu);
    sendButton->SetWidth(120);

    inputRow->AddChild(chatInput, 0, 0);
    inputRow->AddChild(sendButton, 0, 1);
    inputBorder->SetChild(inputRow);

    mainGrid->AddChild(headerBar, 0, 0);
    mainGrid->AddChild(messageAreaBorder, 1, 0);
    mainGrid->AddChild(inputBorder, 2, 0);

    rootGrid->AddChild(sidebar, 0, 0);
    rootGrid->AddChild(mainGrid, 0, 1);

    window.SetContent(rootGrid);
    window.Resize(980, 640);
    window.Show();

   /* std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        Core::Application::DumpVisualTree(NeoWPF::Diagnostics::VisualTreeDumpOptions(
            NeoWPF::Diagnostics::IncludeLayout |
            NeoWPF::Diagnostics::IncludeProperties |
            NeoWPF::Diagnostics::IncludeStyles |
            NeoWPF::Diagnostics::IncludeDiagnostics));
    }).detach(); */

    int result = Core::Application::Run();
    Core::Application::Shutdown();
    return result;
}
