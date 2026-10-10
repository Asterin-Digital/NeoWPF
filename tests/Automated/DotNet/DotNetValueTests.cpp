#include <NeoWPF/DotNet/Errors/ManagedResult.h>
#include <NeoWPF/DotNet/References/ManagedReference.h>
#include <NeoWPF/DotNet/Values/ManagedDecimal.h>

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

TEST(DotNetValueTests, ManagedResultStoresSuccessAndFailure)
{
    const auto success = NeoWPF::DotNet::ManagedResult<int>::Success(42);
    ASSERT_TRUE(success.Succeeded());
    ASSERT_TRUE(success.GetValue().has_value());
    EXPECT_EQ(*success.GetValue(), 42);

    const auto failure = NeoWPF::DotNet::ManagedResult<int>::Error("failed");
    EXPECT_FALSE(failure.Succeeded());
    EXPECT_EQ(failure.GetMessage(), "failed");

    const auto emptySuccess = NeoWPF::DotNet::ManagedResult<void>::Success();
    EXPECT_TRUE(emptySuccess.Succeeded());
}

TEST(DotNetValueTests, ManagedReferencePreservesAbiFields)
{
    const NeoWPF::DotNet::ManagedObjectReference reference(3, 7, 2);

    EXPECT_FALSE(reference.IsNull());
    EXPECT_EQ(reference.GetContextId(), 3);
    EXPECT_EQ(reference.GetIndex(), 7);
    EXPECT_EQ(reference.GetGeneration(), 2);
    EXPECT_EQ(reference.GetKind(), NeoWPF::DotNet::ManagedReferenceKind::Object);
    EXPECT_TRUE(NeoWPF::DotNet::ManagedObjectReference().IsNull());
}

TEST(DotNetValueTests, ManagedDecimalPreservesAbiWords)
{
    const NeoWPF::DotNet::ManagedDecimal decimal(1, 2, 3, 4);

    EXPECT_EQ(decimal.GetBits(), (std::array<std::int32_t, 4>{ 1, 2, 3, 4 }));
}

}