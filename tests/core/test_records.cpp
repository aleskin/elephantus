#include "core/party/records.hpp"
#include "core/settings/user_files.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using ce::core::KvError;
using ce::core::parse_kv;
using ce::core::Records;

TEST(Records, OnlyABetterScoreIsKept)
{
    Records records;
    EXPECT_EQ(records.best("circus"), 0);
    EXPECT_FALSE(records.offer("circus", 0));
    EXPECT_TRUE(records.offer("circus", 10));
    EXPECT_FALSE(records.offer("circus", 5));
    EXPECT_FALSE(records.offer("circus", 10));
    EXPECT_TRUE(records.offer("circus", 15));
    EXPECT_EQ(records.best("circus"), 15);
    EXPECT_EQ(records.best("ball"), 0);
}

TEST(Records, FormatThenParseGivesTheSame)
{
    Records records;
    records.offer("circus", 15);
    records.offer("ball", 3);
    const Records read = ce::core::parse_records(parse_kv(ce::core::format_records(records)));
    EXPECT_EQ(read.all(), records.all());
}

TEST(Records, BrokenFileIsAnError)
{
    EXPECT_THROW(ce::core::parse_records(parse_kv("best = 5\n")), KvError);
    EXPECT_THROW(ce::core::parse_records(parse_kv("[circus]\n")), KvError);
    EXPECT_THROW(ce::core::parse_records(parse_kv("[circus]\nbest = -1\n")), KvError);
    EXPECT_THROW(ce::core::parse_records(parse_kv("[circus]\nbest = many\n")), KvError);
}

TEST(Records, LoadsWhatWasWrittenAndNothingWithoutAFile)
{
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / "elephantus_test_records";
    std::filesystem::remove_all(dir);
    EXPECT_TRUE(ce::core::load_records(dir / "records.txt").all().empty());
    Records records;
    records.offer("circus", 20);
    ce::core::write_text_file(dir / "records.txt", ce::core::format_records(records));
    EXPECT_EQ(ce::core::load_records(dir / "records.txt").best("circus"), 20);
    std::filesystem::remove_all(dir);
}
