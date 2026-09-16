#include <gtest/gtest.h>

#include <NeoWPF/Contracts/IFileDialog.h>
#include <NeoWPF/Platform/Windows/WFileDialog.h>

#include <string>
#include <vector>

namespace {

TEST(FileDialogTests, BuildsFilterSpecsFromExtensions)
{
    const std::vector<NeoWPF::Contracts::FileDialogFilter> filters = {
        {L"Images", {L".png", L".jpg", L".jpeg"}},
        {L"Text", {L".txt"}},
    };

    const auto specs = NeoWPF::Platform::Windows::WFileDialog::BuildFilterSpecs(filters);

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
    const auto specs = NeoWPF::Platform::Windows::WFileDialog::BuildFilterSpecs({});

    ASSERT_EQ(specs.size(), 1u);
    EXPECT_EQ(specs[0].Name, L"All Files");
    EXPECT_EQ(specs[0].Extensions.size(), 1u);
    EXPECT_EQ(specs[0].Extensions[0], L"*");
}

} // namespace
