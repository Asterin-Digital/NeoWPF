#include <gtest/gtest.h>

#include <NeoWPF/Etc/Result.h>

#include <memory>
#include <string>
#include <variant>

namespace {

TEST(ResultTests, SuccessValueSetsSuccessState)
{
    const NeoWPF::Result<int> result = NeoWPF::Success{42};

    EXPECT_TRUE(result.Success());
    EXPECT_FALSE(result.Fail());
    EXPECT_EQ(result.Value(), 42);
}

TEST(ResultTests, FailureErrorSetsFailureState)
{
    const NeoWPF::Result<int> result = NeoWPF::Fail{std::string("failed")};

    EXPECT_FALSE(result.Success());
    EXPECT_TRUE(result.Fail());
    EXPECT_EQ(result.Error(), "failed");
}

TEST(ResultTests, BoolPayloadIsIndependentOfSuccessState)
{
    const NeoWPF::Result<bool> success = NeoWPF::Success{false};

    EXPECT_TRUE(success.Success());
    EXPECT_FALSE(success.Fail());
    EXPECT_FALSE(success.Value());
}

TEST(ResultTests, VoidResultRepresentsSuccessAndFailure)
{
    const NeoWPF::Result<void> success = NeoWPF::Success<void>{};
    const NeoWPF::Result<void> failure = NeoWPF::Fail{std::string("failed")};

    EXPECT_TRUE(success.Success());
    EXPECT_FALSE(success.Fail());
    EXPECT_NO_THROW(success.Value());

    EXPECT_FALSE(failure.Success());
    EXPECT_TRUE(failure.Fail());
    EXPECT_EQ(failure.Error(), "failed");
}

TEST(ResultTests, MoveOnlyValueIsSupported)
{
    NeoWPF::Result<std::unique_ptr<int>> result =
        NeoWPF::Success{std::make_unique<int>(42)};

    ASSERT_TRUE(result.Success());
    ASSERT_NE(result.Value(), nullptr);
    EXPECT_EQ(*result.Value(), 42);
}

TEST(ResultTests, AccessingInactiveAlternativeThrows)
{
    const NeoWPF::Result<int> success = NeoWPF::Success{42};
    const NeoWPF::Result<int> failure = NeoWPF::Fail{std::string("failed")};

    EXPECT_THROW(failure.Value(), std::bad_variant_access);
    EXPECT_THROW(success.Error(), std::bad_variant_access);
}

}
