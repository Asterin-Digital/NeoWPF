#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>

#include <algorithm>
#include <array>
#include <memory>
#include <string>

namespace {

class EmbeddedResourceProviderTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        NeoWPF::EmbeddedResourceProvider::Instance().Clear();
    }

    void TearDown() override
    {
        NeoWPF::EmbeddedResourceProvider::Instance().Clear();
    }
};

TEST_F(EmbeddedResourceProviderTest, RegisterCopiesBytesAndPreservesResourceType)
{
    std::array<unsigned char, 4> source = {0x00, 0x7F, 0x80, 0xFF};
    const std::array<unsigned char, 4> expected = source;
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();

    provider.Register(NeoWPF::ResourceKey("images/icon.bin"), NeoWPF::ResourceType::Image,
                      source.data(), source.size());
    source.fill(0x11);

    NeoWPF::ResourceBlob blob;
    ASSERT_TRUE(provider.TryGet(NeoWPF::ResourceKey("images/icon.bin"), blob));
    ASSERT_NE(blob.Data, nullptr);
    EXPECT_EQ(blob.Size, expected.size());
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image);
    EXPECT_NE(blob.Owner, nullptr);
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), blob.Data));
}

TEST_F(EmbeddedResourceProviderTest, RegisterReplacesExistingResource)
{
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();
    const std::array<unsigned char, 2> firstData = {1, 2};
    const std::array<unsigned char, 3> secondData = {3, 4, 5};
    const NeoWPF::ResourceKey key("replace.bin");

    provider.Register(key, NeoWPF::ResourceType::Raw, firstData.data(), firstData.size());
    provider.Register(key, NeoWPF::ResourceType::Font, secondData.data(), secondData.size());

    NeoWPF::ResourceBlob blob;
    ASSERT_TRUE(provider.TryGet(key, blob));
    ASSERT_NE(blob.Data, nullptr);
    EXPECT_EQ(blob.Size, secondData.size());
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Font);
    EXPECT_TRUE(std::equal(secondData.begin(), secondData.end(), blob.Data));
}

TEST_F(EmbeddedResourceProviderTest, InvalidRegistrationDoesNotReplaceOrCreateAResource)
{
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();
    const std::array<unsigned char, 2> originalData = {8, 9};
    const std::array<unsigned char, 2> replacementData = {4, 5};
    const NeoWPF::ResourceKey existingKey("existing.bin");
    const NeoWPF::ResourceKey missingKey("missing.bin");
    provider.Register(existingKey, NeoWPF::ResourceType::Raw, originalData.data(), originalData.size());

    provider.Register(existingKey, NeoWPF::ResourceType::Image, nullptr, replacementData.size());
    provider.Register(existingKey, NeoWPF::ResourceType::Image, replacementData.data(), 0);
    provider.Register(missingKey, NeoWPF::ResourceType::Raw, nullptr, replacementData.size());
    provider.Register(missingKey, NeoWPF::ResourceType::Raw, replacementData.data(), 0);

    NeoWPF::ResourceBlob existingBlob;
    ASSERT_TRUE(provider.TryGet(existingKey, existingBlob));
    EXPECT_EQ(existingBlob.Type, NeoWPF::ResourceType::Raw);
    EXPECT_EQ(existingBlob.Size, originalData.size());
    EXPECT_TRUE(std::equal(originalData.begin(), originalData.end(), existingBlob.Data));

    NeoWPF::ResourceBlob missingBlob;
    EXPECT_FALSE(provider.TryGet(missingKey, missingBlob));
}

TEST_F(EmbeddedResourceProviderTest, UnregisterRemovesOnlyTheRequestedResource)
{
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();
    const std::array<unsigned char, 1> data = {0x2A};
    const NeoWPF::ResourceKey removedKey("removed.bin");
    const NeoWPF::ResourceKey retainedKey("retained.bin");
    provider.Register(removedKey, NeoWPF::ResourceType::Raw, data.data(), data.size());
    provider.Register(retainedKey, NeoWPF::ResourceType::Raw, data.data(), data.size());

    provider.Unregister(removedKey);
    provider.Unregister(removedKey);

    NeoWPF::ResourceBlob blob;
    EXPECT_FALSE(provider.TryGet(removedKey, blob));
    EXPECT_TRUE(provider.TryGet(retainedKey, blob));
}

TEST_F(EmbeddedResourceProviderTest, ClearRemovesEveryRegisteredResource)
{
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();
    const std::array<unsigned char, 1> data = {0x2A};
    const NeoWPF::ResourceKey firstKey("first.bin");
    const NeoWPF::ResourceKey secondKey("second.bin");
    provider.Register(firstKey, NeoWPF::ResourceType::Raw, data.data(), data.size());
    provider.Register(secondKey, NeoWPF::ResourceType::Raw, data.data(), data.size());

    provider.Clear();

    NeoWPF::ResourceBlob blob;
    EXPECT_FALSE(provider.TryGet(firstKey, blob));
    EXPECT_FALSE(provider.TryGet(secondKey, blob));
}

TEST_F(EmbeddedResourceProviderTest, CanRegisterAgainAfterClearAndUnregister)
{
    auto& provider = NeoWPF::EmbeddedResourceProvider::Instance();
    const std::array<unsigned char, 1> data = {0x6D};
    const NeoWPF::ResourceKey key("reusable.bin");
    NeoWPF::ResourceBlob blob;

    provider.Register(key, NeoWPF::ResourceType::Raw, data.data(), data.size());
    provider.Clear();
    EXPECT_FALSE(provider.TryGet(key, blob));

    provider.Register(key, NeoWPF::ResourceType::Raw, data.data(), data.size());
    ASSERT_TRUE(provider.TryGet(key, blob));
    provider.Unregister(key);
    EXPECT_FALSE(provider.TryGet(key, blob));

    provider.Register(key, NeoWPF::ResourceType::Raw, data.data(), data.size());
    ASSERT_TRUE(provider.TryGet(key, blob));
    ASSERT_NE(blob.Data, nullptr);
    EXPECT_EQ(blob.Size, data.size());
    EXPECT_EQ(blob.Data[0], data[0]);
}

}