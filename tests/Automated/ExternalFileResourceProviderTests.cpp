#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Resources/ExternalFileResourceProvider.h>

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <system_error>

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory()
    {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
            ("NeoWPF.ExternalFileResourceProviderTests-" + std::to_string(timestamp) + "-" +
             std::to_string(s_nextId.fetch_add(1)));
        std::filesystem::create_directories(m_path);
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    const std::filesystem::path& Path() const
    {
        return m_path;
    }

    bool WriteFile(const std::filesystem::path& path, const std::string& content) const
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        return file.good();
    }

private:
    inline static std::atomic_uint64_t s_nextId{0};
    std::filesystem::path m_path;
};

void ExpectBlobContents(const NeoWPF::ResourceBlob& blob, const std::string& expected)
{
    ASSERT_NE(blob.Data, nullptr);
    EXPECT_EQ(blob.Size, expected.size());
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Raw);
    EXPECT_NE(blob.Owner, nullptr);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(blob.Data), blob.Size), expected);
}

TEST(ExternalFileResourceProviderTests, LoadsResourcesDirectoryBeforeOtherCandidatesAndOwnsBytes)
{
    TemporaryDirectory directory;
    const std::string selectedContent("resources\0payload", 17);
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "Resources" / "asset.bin", selectedContent));
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "Resources" / "Assets" / "asset.bin", "assets"));
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "asset.bin", "root"));

    NeoWPF::ResourceBlob blob;
    {
        NeoWPF::ExternalFileResourceProvider provider(directory.Path());
        ASSERT_TRUE(provider.TryGet(NeoWPF::ResourceKey("asset.bin"), blob));
    }

    std::error_code error;
    std::filesystem::remove(directory.Path() / "Resources" / "asset.bin", error);
    ExpectBlobContents(blob, selectedContent);
}

TEST(ExternalFileResourceProviderTests, FallsBackToResourcesAssetsDirectory)
{
    TemporaryDirectory directory;
    const std::string expected("asset-data");
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "Resources" / "Assets" / "icon.dat", expected));

    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    NeoWPF::ResourceBlob blob;
    ASSERT_TRUE(provider.TryGet(NeoWPF::ResourceKey("icon.dat"), blob));

    ExpectBlobContents(blob, expected);
}

TEST(ExternalFileResourceProviderTests, FallsBackToBaseDirectory)
{
    TemporaryDirectory directory;
    const std::string expected("root-resource");
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "font.bin", expected));

    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    NeoWPF::ResourceBlob blob;
    ASSERT_TRUE(provider.TryGet(NeoWPF::ResourceKey("font.bin"), blob));

    ExpectBlobContents(blob, expected);
}

TEST(ExternalFileResourceProviderTests, MissingBaseDirectoryLeavesOutputUnchanged)
{
    NeoWPF::ExternalFileResourceProvider provider({});
    unsigned char sentinel = 0xA5;
    NeoWPF::ResourceBlob blob;
    blob.Data = &sentinel;
    blob.Size = 23;
    blob.Type = NeoWPF::ResourceType::Image;
    blob.Owner = std::make_shared<int>(42);
    const auto owner = blob.Owner;

    EXPECT_FALSE(provider.TryGet(NeoWPF::ResourceKey("missing.bin"), blob));
    EXPECT_EQ(blob.Data, &sentinel);
    EXPECT_EQ(blob.Size, 23u);
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image);
    EXPECT_EQ(blob.Owner, owner);
}

TEST(ExternalFileResourceProviderTests, MissingFileLeavesOutputUnchanged)
{
    TemporaryDirectory directory;
    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    unsigned char sentinel = 0xA5;
    NeoWPF::ResourceBlob blob;
    blob.Data = &sentinel;
    blob.Size = 23;
    blob.Type = NeoWPF::ResourceType::Image;
    blob.Owner = std::make_shared<int>(42);
    const auto owner = blob.Owner;

    EXPECT_FALSE(provider.TryGet(NeoWPF::ResourceKey("missing.bin"), blob));
    EXPECT_EQ(blob.Data, &sentinel);
    EXPECT_EQ(blob.Size, 23u);
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image);
    EXPECT_EQ(blob.Owner, owner);
}

TEST(ExternalFileResourceProviderTests, RejectsEmptyFilesAndLeavesOutputUnchanged)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "Resources" / "empty.bin", ""));
    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    unsigned char sentinel = 0xA5;
    NeoWPF::ResourceBlob blob;
    blob.Data = &sentinel;
    blob.Size = 23;
    blob.Type = NeoWPF::ResourceType::Image;
    blob.Owner = std::make_shared<int>(42);
    const auto owner = blob.Owner;

    EXPECT_FALSE(provider.TryGet(NeoWPF::ResourceKey("empty.bin"), blob));
    EXPECT_EQ(blob.Data, &sentinel);
    EXPECT_EQ(blob.Size, 23u);
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image);
    EXPECT_EQ(blob.Owner, owner);
}

TEST(ExternalFileResourceProviderTests, RejectsExternalNamlFilesAndLeavesOutputUnchanged)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.WriteFile(directory.Path() / "Resources" / "view.naml", "markup"));
    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    unsigned char sentinel = 0xA5;
    NeoWPF::ResourceBlob blob;
    blob.Data = &sentinel;
    blob.Size = 23;
    blob.Type = NeoWPF::ResourceType::Image;
    blob.Owner = std::make_shared<int>(42);
    const auto owner = blob.Owner;

    EXPECT_FALSE(provider.TryGet(NeoWPF::ResourceKey("view.naml"), blob));
    EXPECT_EQ(blob.Data, &sentinel);
    EXPECT_EQ(blob.Size, 23u);
    EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image);
    EXPECT_EQ(blob.Owner, owner);
}

TEST(ExternalFileResourceProviderTests, ReflectsFileChangesAndRemovalAcrossLookups)
{
    TemporaryDirectory directory;
    const auto resourcePath = directory.Path() / "Resources" / "live.bin";
    ASSERT_TRUE(directory.WriteFile(resourcePath, "first-version"));
    NeoWPF::ExternalFileResourceProvider provider(directory.Path());
    const NeoWPF::ResourceKey key("live.bin");
    NeoWPF::ResourceBlob blob;

    ASSERT_TRUE(provider.TryGet(key, blob));
    ExpectBlobContents(blob, "first-version");

    ASSERT_TRUE(directory.WriteFile(resourcePath, "second-version"));
    ASSERT_TRUE(provider.TryGet(key, blob));
    ExpectBlobContents(blob, "second-version");

    std::error_code error;
    ASSERT_TRUE(std::filesystem::remove(resourcePath, error));
    ASSERT_FALSE(error);
    const auto previousData = blob.Data;
    const auto previousSize = blob.Size;
    const auto previousOwner = blob.Owner;

    EXPECT_FALSE(provider.TryGet(key, blob));
    EXPECT_EQ(blob.Data, previousData);
    EXPECT_EQ(blob.Size, previousSize);
    EXPECT_EQ(blob.Owner, previousOwner);
}

TEST(ExternalFileResourceProviderTests, RejectsParentTraversalAndAbsolutePathsOutsideBaseDirectory)
{
    TemporaryDirectory directory;
    const auto baseDirectory = directory.Path() / "base";
    const auto outsidePath = directory.Path() / "outside.bin";
    ASSERT_TRUE(directory.WriteFile(outsidePath, "outside-data"));
    NeoWPF::ExternalFileResourceProvider provider(baseDirectory);

    unsigned char sentinel = 0xA5;
    NeoWPF::ResourceBlob blob;
    blob.Data = &sentinel;
    blob.Size = 17;
    blob.Type = NeoWPF::ResourceType::Image;
    blob.Owner = std::make_shared<int>(42);
    const auto owner = blob.Owner;

    const std::array<std::string, 2> hostilePaths = {
        "../outside.bin",
        outsidePath.generic_string()
    };
    for (const auto& hostilePath : hostilePaths) {
        EXPECT_FALSE(provider.TryGet(NeoWPF::ResourceKey(hostilePath.c_str()), blob)) << hostilePath;
        EXPECT_EQ(blob.Data, &sentinel) << hostilePath;
        EXPECT_EQ(blob.Size, 17u) << hostilePath;
        EXPECT_EQ(blob.Type, NeoWPF::ResourceType::Image) << hostilePath;
        EXPECT_EQ(blob.Owner, owner) << hostilePath;
    }
}

}