#include <gtest/gtest.h>

#include <NeoWPF/Contracts/IGeneral.h>
#if defined(_WIN32)
#include <NeoWPF/Platform/Windows/WFileDialog.h>
#else
#include <NeoWPF/Platform/Linux/LFileDialog.h>
#endif

#include <string>
#include <vector>

namespace {

#if defined(_WIN32)
using FileDialog = NeoWPF::Platform::Windows::WFileDialog;
#else
using FileDialog = NeoWPF::Platform::Linux::LFileDialog;
#endif

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
