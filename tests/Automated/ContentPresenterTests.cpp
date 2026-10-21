#include <gtest/gtest.h>

#include <any>

#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>

TEST(ContentPresenterTests, SetContentData_AttachesDerivedUIElementSharedPointerAsContent)
{
    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    auto content = std::make_shared<NeoWPF::UI::TextBlock>(L"content");

    presenter->SetContentData(std::any(content));
    presenter->Measure(100.0, 100.0);

    EXPECT_EQ(content->GetParent(), presenter.get());
}
