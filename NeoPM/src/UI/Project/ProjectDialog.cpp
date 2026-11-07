#include <NeoPM/UI/Project/ProjectDialog.h>
#include <NeoPM/UI/Components/Dialog.h>
#include <NeoPM/UI/Components/MakeButton.h>
#include <NeoPM/UI/Components/Theme.h>
#include <NeoPM/Services/ProjectService.h>
#include <NeoPM/Utils/TextEncoding.h>

#include <NeoWPF/NeoWPF.h>

#include <any>

namespace NeoPM::UI::Project {

using namespace NeoWPF::UI;

namespace {

Ref<TextBox> AddTextField(
    const Ref<StackPanel>& panel,
    const std::wstring& label,
    const std::wstring& placeholder)
{
    auto labelBlock = New<TextBlock>(label);
    labelBlock->SetTextColor(Theme::TextMuted);
    labelBlock->SetFontSize(13);
    panel->AddChild(labelBlock);

    auto textBox = New<TextBox>();
    textBox->SetPlaceholderText(placeholder);
    textBox->SetHeight(32);
    textBox->SetMargin(Thickness(0, 3, 0, 8));
    panel->AddChild(textBox);
    return textBox;
}

Ref<ComboBox> AddComboField(
    const Ref<StackPanel>& panel,
    const std::wstring& label,
    const std::vector<std::wstring>& values)
{
    auto labelBlock = New<TextBlock>(label);
    labelBlock->SetTextColor(Theme::TextMuted);
    labelBlock->SetFontSize(13);
    panel->AddChild(labelBlock);

    auto comboBox = New<ComboBox>();
    std::vector<std::any> items;
    for (const auto& value : values) {
        items.emplace_back(value);
    }
    comboBox->SetItemsSource(items);
    comboBox->SetSelectedIndex(0);
    comboBox->SetHeight(32);
    comboBox->SetMargin(Thickness(0, 3, 0, 8));
    panel->AddChild(comboBox);
    return comboBox;
}

std::string SelectedValue(const Ref<ComboBox>& comboBox)
{
    return Utils::WideToUtf8(
        std::any_cast<std::wstring>(comboBox->GetSelectedItem()));
}

std::optional<std::string> OptionalValue(const Ref<TextBox>& textBox)
{
    if (textBox->GetText().empty()) {
        return std::nullopt;
    }
    return Utils::WideToUtf8(textBox->GetText());
}

}

void ShowProjectDialog(
    FrameworkElement* placementTarget,
    PopupPlacement placement)
{
    auto content = New<StackPanel>();
    content->SetBackgroundColor(Theme::Surface);
    content->SetPadding(Thickness(24));
    content->SetWidth(520);

    auto title = New<TextBlock>(L"Create a new project");
    title->SetFontSize(22);
    title->SetFontWeight(700);
    title->SetTextColor(Theme::TextPrimary);
    title->SetMargin(Thickness(0, 0, 0, 16));
    content->AddChild(title);

    auto name = AddTextField(content, L"Project name", L"MyProject");
    auto outputPath = AddTextField(content, L"Parent path", L"Optional");
    auto sdkSource = AddComboField(content, L"SDK source", {L"production", L"user"});
    auto sdkConfiguration = AddComboField(content, L"SDK configuration", {L"debug", L"release"});
    auto sdkIncludePath = AddTextField(content, L"SDK include root", L"Contains NeoWPF and NeoNaml");
    auto sdkLibraryPath = AddTextField(content, L"SDK library root", L"Contains Debug and Release");

    auto actions = New<StackPanel>();
    actions->SetOrientation(Orientation::Horizontal);
    actions->SetHorizontalAlignment(HorizontalAlignment::Right);

    auto cancel = Components::MakeButton(L"Cancel");
    cancel->SetWidth(110);
    cancel->SetMargin(Thickness(0, 0, 10, 0));
    auto create = Components::MakeButton(L"Create project");
    create->SetWidth(150);
    actions->AddChild(cancel);
    actions->AddChild(create);
    content->AddChild(actions);

    auto dialog = Components::MakeDialog(placementTarget, content, placement);
    cancel->Click += [dialog]() {
        dialog->SetIsOpen(false);
    };
    create->Click += [
        dialog,
        name,
        outputPath,
        sdkSource,
        sdkConfiguration,
        sdkIncludePath,
        sdkLibraryPath
    ]() {
        Services::ProjectCreationRequest request{
            Utils::WideToUtf8(name->GetText()),
            OptionalValue(outputPath),
            SelectedValue(sdkSource),
            SelectedValue(sdkConfiguration),
            OptionalValue(sdkIncludePath),
            OptionalValue(sdkLibraryPath)
        };

        if (Services::ProjectService::Create(request) == 0) {
            dialog->SetIsOpen(false);
        }
    };
}

}
