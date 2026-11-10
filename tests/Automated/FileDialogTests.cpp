#include <gtest/gtest.h>

#include <NeoWPF/Contracts/IGeneral.h>
#include <NeoWPF/Platform/Services/FilePicker.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#if defined(_WIN32)
#include <NeoWPF/Platform/Windows/WFileDialog.h>
#else
#include <NeoWPF/Platform/Linux/LFileDialog.h>
#endif

#include <string>
#include <type_traits>
#include <vector>

namespace {

#if defined(_WIN32)
using FileDialog = NeoWPF::Platform::Windows::WFileDialog;
#else
using FileDialog = NeoWPF::Platform::Linux::LFileDialog;
#endif

static_assert(!std::is_base_of_v<NeoWPF::UI::Button, NeoWPF::Platform::Services::FilePicker>);

TEST(FileDialogTests, BuildsFilterSpecsFromExtensions)
{
    const std::vector<NeoWPF::Contracts::FileDialogFilter> filters = {
        {L"Images", {L".png", L".jpg", L".jpeg"}},
        {L"Text", {L".txt"}},
    };

    const auto specs = FileDialog::BuildFilterSpecs(filters);

    ASSERT_EQ(specs.size(), 2u);
    EXPECT_EQ(specs[0].Name, L"Images");
    EXPECT_EQ(specs[0].Extensions.size(), 3u);
    EXPECT_EQ(specs[0].Extensions[0], L".png");
    EXPECT_EQ(specs[0].Extensions[1], L".jpg");
    EXPECT_EQ(specs[0].Extensions[2], L".jpeg");
    EXPECT_EQ(specs[1].Name, L"Text");
    EXPECT_EQ(specs[1].Extensions.size(), 1u);
    EXPECT_EQ(specs[1].Extensions[0], L".txt");
}

TEST(FileDialogTests, FilePickerStoresDialogOptions)
{
    NeoWPF::Platform::Services::FilePicker picker;
    const std::vector<NeoWPF::Contracts::FileDialogFilter> filters = {
        {L"Text", {L"txt"}},
    };

    picker.SetTitle(L"Open notes");
    picker.SetInitialDirectory(L"C:\\Documents");
    picker.SetDefaultFileName(L"notes.txt");
    picker.SetAllowMultiSelect(true);
    picker.SetDialogMode(NeoWPF::Platform::Services::FilePickerMode::Save);
    picker.SetFilters(filters);

    EXPECT_EQ(picker.GetTitle(), L"Open notes");
    EXPECT_EQ(picker.GetInitialDirectory(), L"C:\\Documents");
    EXPECT_EQ(picker.GetDefaultFileName(), L"notes.txt");
    EXPECT_TRUE(picker.GetAllowMultiSelect());
    EXPECT_EQ(picker.GetDialogMode(), NeoWPF::Platform::Services::FilePickerMode::Save);
    const auto& actualFilters = picker.GetFilters();
    ASSERT_EQ(actualFilters.size(), 1u);
    EXPECT_EQ(actualFilters[0].Name, L"Text");
    ASSERT_EQ(actualFilters[0].Extensions.size(), 1u);
    EXPECT_EQ(actualFilters[0].Extensions[0], L"txt");
}

TEST(FileDialogTests, BuildsAllFilesFallbackWhenNoFiltersAreProvided)
{
    const auto specs = FileDialog::BuildFilterSpecs({});

    ASSERT_EQ(specs.size(), 1u);
    EXPECT_EQ(specs[0].Name, L"All Files");
    EXPECT_EQ(specs[0].Extensions.size(), 1u);
    EXPECT_EQ(specs[0].Extensions[0], L"*");
}

TEST(FileDialogTests, CancellationIsSuccessfulWithoutASelection)
{
    NeoWPF::Contracts::FileDialogResult result =
        NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};

    ASSERT_TRUE(result.Success());
    EXPECT_FALSE(result.Value().has_value());
}

TEST(FileDialogTests, MissingClipboardTextIsSuccessfulWithoutAValue)
{
    NeoWPF::Contracts::ClipboardTextResult result =
        NeoWPF::Success<std::optional<std::wstring>>{std::nullopt};

    ASSERT_TRUE(result.Success());
    EXPECT_FALSE(result.Value().has_value());
}

} // namespace
