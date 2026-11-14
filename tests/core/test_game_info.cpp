#include "core/game_info.hpp"

#include <gtest/gtest.h>

#include <regex>
#include <string>

TEST(GameInfo, TitleIsCircusElephant)
{
    EXPECT_EQ(ce::core::game_title(), "Circus Elephant");
}

TEST(GameInfo, VersionIsMajorMinorPatchFromMakefileVersion)
{
    const std::string version{ce::core::game_version()};
    EXPECT_EQ(version, CE_VERSION);
    EXPECT_TRUE(std::regex_match(version, std::regex{R"(\d+\.\d+\.\d+)"})) << version;
}
