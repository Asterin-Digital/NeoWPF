#include <gtest/gtest.h>

#include <NeoPM/Utils/Guid.h>

#include <regex>

TEST(GuidTests, GeneratesUppercaseDashedVersionFourUuid)
{
    const std::regex uuidPattern(
        "^[0-9A-F]{8}-[0-9A-F]{4}-4[0-9A-F]{3}-[89AB][0-9A-F]{3}-[0-9A-F]{12}$");

    EXPECT_TRUE(std::regex_match(NeoPM::Utils::GenerateGuid(), uuidPattern));
}

TEST(GuidTests, GeneratesDistinctIdentifiers)
{
    EXPECT_NE(NeoPM::Utils::GenerateGuid(), NeoPM::Utils::GenerateGuid());
}