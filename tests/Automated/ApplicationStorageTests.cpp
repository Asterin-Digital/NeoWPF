#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <filesystem>
#include <memory>
#include <system_error>

namespace {

class PlatformReset final {
public:
    ~PlatformReset()
    {
        NeoWPF::Platform::RegisterPlatform(nullptr);
    }
};

class StorageDirectoryCleanup final {
public:
    ~StorageDirectoryCleanup()
    {
        if (!m_path.empty()) {
            std::error_code error;
            std::filesystem::remove_all(m_path, error);
        }
    }

    void SetPath(std::filesystem::path path)
    {
        m_path = std::move(path);
    }

private:
    std::filesystem::path m_path;
};

}

TEST(ApplicationStorageTests, AcceptsUppercaseAndLowercaseDashedUuid)
{
    EXPECT_TRUE(NeoWPF::Platform::IsValidUuid("A1B2C3D4-E5F6-4789-ABCD-0123456789AB"));
    EXPECT_TRUE(NeoWPF::Platform::IsValidUuid("a1b2c3d4-e5f6-4789-abcd-0123456789ab"));
}

TEST(ApplicationStorageTests, RejectsInvalidUuidLengthAndSeparators)
{
    EXPECT_FALSE(NeoWPF::Platform::IsValidUuid("A1B2C3D4-E5F6-4789-ABCD-0123456789A"));
    EXPECT_FALSE(NeoWPF::Platform::IsValidUuid("A1B2C3D4_E5F6-4789-ABCD-0123456789AB"));
}

TEST(ApplicationStorageTests, RejectsNonHexadecimalUuidCharacters)
{
    EXPECT_FALSE(NeoWPF::Platform::IsValidUuid("A1B2C3D4-E5F6-4789-ABCD-0123456789AG"));
}

TEST(ApplicationStorageTests, RejectsInvalidIdentifierBeforePlatformLookup)
{
    const auto result = NeoWPF::Core::Application::GetStorageDirectory("not-a-uuid");

    EXPECT_TRUE(result.Fail());
}

TEST(ApplicationStorageTests, CreatesStorageDirectoryAndCachesSuccessfulResolution)
{
    PlatformReset platformReset;
    StorageDirectoryCleanup cleanup;
    auto platform = NeoWPF::Platform::CreateDefaultPlatform();
    ASSERT_NE(platform, nullptr);
    NeoWPF::Platform::RegisterPlatform(std::move(platform));

    const std::string uuid = "A1B2C3D4-E5F6-4789-ABCD-0123456789AC";
    const auto result = NeoWPF::Core::Application::GetStorageDirectory(uuid);
    ASSERT_TRUE(result.Success());

    const auto storageDirectory = result.Value();
    cleanup.SetPath(storageDirectory);
    EXPECT_TRUE(storageDirectory.is_absolute());
    EXPECT_EQ(storageDirectory.filename(), uuid);
    EXPECT_EQ(storageDirectory.parent_path().filename(), "Apps");
    EXPECT_EQ(storageDirectory.parent_path().parent_path().filename(), "Asterin Digital");
    EXPECT_TRUE(std::filesystem::is_directory(storageDirectory));

    std::error_code error;
    std::filesystem::remove_all(storageDirectory, error);
    ASSERT_FALSE(error);

    const auto cachedResult = NeoWPF::Core::Application::GetStorageDirectory(uuid);
    ASSERT_TRUE(cachedResult.Success());
    EXPECT_EQ(cachedResult.Value(), storageDirectory);
    EXPECT_FALSE(std::filesystem::exists(storageDirectory));
}