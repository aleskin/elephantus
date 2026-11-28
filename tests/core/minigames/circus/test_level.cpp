#include "core/minigames/circus/level/level.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <variant>

using ce::core::KvError;
using ce::core::parse_kv;
using ce::core::circus::Hoop;
using ce::core::circus::LevelData;
using ce::core::circus::Podium;

namespace
{

LevelData level_from(std::string_view text)
{
    return ce::core::circus::parse_level(parse_kv(text));
}

int error_line(std::string_view text)
{
    try
    {
        level_from(text);
    }
    catch (const KvError& error)
    {
        return error.line();
    }
    return -1;
}

const std::filesystem::path levels_dir =
    std::filesystem::path{CE_ASSETS_DIR} / "minigames" / "circus" / "levels";

} // namespace

TEST(Level, ParsesHeaderAndHoopsSortedByX)
{
    const LevelData level = level_from("[level]\n"
                                       "title = Test\n"
                                       "location = arena\n"
                                       "length = 50\n"
                                       "start = 1,5\n"
                                       "[hoop]\nx = 30\n"
                                       "[hoop]\nx = 10\nheight = 2\nradius = 0.8\n");
    EXPECT_EQ(level.title, "Test");
    EXPECT_EQ(level.location, "arena");
    EXPECT_DOUBLE_EQ(level.length, 50.0);
    EXPECT_DOUBLE_EQ(level.start_x, 1.5);
    ASSERT_EQ(level.objects.size(), 2u);
    const Hoop& first = std::get<Hoop>(level.objects[0]);
    EXPECT_DOUBLE_EQ(first.x, 10.0);
    EXPECT_DOUBLE_EQ(first.height, 2.0);
    EXPECT_DOUBLE_EQ(first.radius, 0.8);
    EXPECT_DOUBLE_EQ(first.bottom(), 1.2);
    const Hoop& second = std::get<Hoop>(level.objects[1]);
    EXPECT_DOUBLE_EQ(second.x, 30.0);
    EXPECT_DOUBLE_EQ(second.height, Hoop::default_height);
    EXPECT_DOUBLE_EQ(second.radius, Hoop::default_radius);
}

TEST(Level, LevelWithoutObjectsIsValid)
{
    const LevelData level = level_from("[level]\nlocation = arena\nlength = 10\n");
    EXPECT_TRUE(level.objects.empty());
    EXPECT_DOUBLE_EQ(level.start_x, 0.0);
}

TEST(Level, ErrorsPointAtTheRightLine)
{
    EXPECT_EQ(error_line("[hoop]\nx = 1\n"), 0);
    EXPECT_EQ(error_line("length = 10\n[level]\n"), 1);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[level]\n"), 4);
    EXPECT_EQ(error_line("[level]\nlocation = moon\nlength = 10\n"), 2);
    EXPECT_EQ(error_line("[level]\nlocation = arena\n"), 1);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 0\n"), 3);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\nstart = 10\n"), 4);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[clown]\n"), 4);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[hoop]\nheight = 2\n"), 4);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[hoop]\nx = 11\n"), 5);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[hoop]\nx = 5\nradius = 0\n"), 6);
    EXPECT_EQ(error_line("[level]\nlocation = arena\nlength = 10\n[hoop]\nx = 5\nheight = 0.5\n"),
              4);
}

TEST(Level, ParsesPodium)
{
    const LevelData level = level_from("[level]\nlocation = arena\nlength = 20\n"
                                       "[podium]\nx = 18\nwidth = 3\nheight = 0.8\n");
    ASSERT_EQ(level.objects.size(), 1u);
    const Podium& podium = std::get<Podium>(level.objects[0]);
    EXPECT_DOUBLE_EQ(podium.x, 18.0);
    EXPECT_DOUBLE_EQ(podium.left(), 16.5);
    EXPECT_DOUBLE_EQ(podium.right(), 19.5);
    EXPECT_DOUBLE_EQ(podium.height, 0.8);
}

TEST(Level, PodiumErrorsPointAtTheRightLine)
{
    const std::string head = "[level]\nlocation = arena\nlength = 20\n";
    EXPECT_EQ(error_line(head + "[podium]\nwidth = 2\n"), 4);
    EXPECT_EQ(error_line(head + "[podium]\nx = 10\nwidth = 0\n"), 6);
    EXPECT_EQ(error_line(head + "[podium]\nx = 10\nheight = -1\n"), 6);
    EXPECT_EQ(error_line(head + "[podium]\nx = 19.5\n"), 5);
    EXPECT_EQ(error_line(head + "[podium]\nx = 10\n[podium]\nx = 15\n"), 6);
}

TEST(Level, KnownLocations)
{
    EXPECT_TRUE(ce::core::circus::is_known_location("arena"));
    EXPECT_FALSE(ce::core::circus::is_known_location("Arena"));
    EXPECT_FALSE(ce::core::circus::is_known_location(""));
}

TEST(Campaign, ListsLevelsInOrderRelativeToFolder)
{
    const auto campaign = ce::core::circus::parse_campaign(
        parse_kv("[level]\nfile = a.txt\n[level]\nfile = b.txt\n"), "levels");
    ASSERT_EQ(campaign.levels.size(), 2u);
    EXPECT_EQ(campaign.levels[0], std::filesystem::path{"levels"} / "a.txt");
    EXPECT_EQ(campaign.levels[1], std::filesystem::path{"levels"} / "b.txt");
}

TEST(Campaign, Errors)
{
    EXPECT_THROW(ce::core::circus::parse_campaign(parse_kv("# empty\n"), "."), KvError);
    EXPECT_THROW(ce::core::circus::parse_campaign(parse_kv("[level]\n"), "."), KvError);
    EXPECT_THROW(ce::core::circus::parse_campaign(parse_kv("[stage]\nfile = a.txt\n"), "."),
                 KvError);
}

TEST(Level, LoadErrorNamesTheFile)
{
    const auto path = std::filesystem::temp_directory_path() / "ce_test_level_bad.txt";
    {
        std::ofstream out{path};
        out << "[level]\nlocation = moon\nlength = 10\n";
    }
    try
    {
        ce::core::circus::load_level(path);
        FAIL() << "expected KvError";
    }
    catch (const KvError& error)
    {
        EXPECT_EQ(std::string{error.what()},
                  "ce_test_level_bad.txt, line 2: unknown location 'moon'");
    }
    std::filesystem::remove(path);
}

TEST(Assets, EveryCampaignLevelLoads)
{
    const auto campaign = ce::core::circus::load_campaign(levels_dir / "campaign.txt");
    ASSERT_FALSE(campaign.levels.empty());
    for (const auto& path : campaign.levels)
    {
        SCOPED_TRACE(path.string());
        const LevelData level = ce::core::circus::load_level(path);
        EXPECT_FALSE(level.title.empty());
    }
}

TEST(Assets, FirstLevelHasThreeHoopsAndThePodiumLast)
{
    const LevelData level = ce::core::circus::load_level(levels_dir / "level_01.txt");
    EXPECT_EQ(level.location, "arena");
    ASSERT_EQ(level.objects.size(), 4u);
    double previous = level.start_x;
    for (std::size_t i = 0; i < 3; ++i)
    {
        const Hoop& hoop = std::get<Hoop>(level.objects[i]);
        EXPECT_GT(hoop.x, previous);
        previous = hoop.x;
    }
    const Podium& podium = std::get<Podium>(level.objects[3]);
    EXPECT_GT(podium.left(), previous);
    EXPECT_LE(podium.right(), level.length);
}
