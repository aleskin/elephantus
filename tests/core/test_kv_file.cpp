#include "core/text/kv_file.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

using ce::core::KvDocument;
using ce::core::KvError;
using ce::core::parse_kv;

namespace
{

int error_line(std::string_view text)
{
    try
    {
        parse_kv(text);
    }
    catch (const KvError& error)
    {
        return error.line();
    }
    return -1;
}

} // namespace

TEST(KvFile, ParsesSectionsKeysAndComments)
{
    const KvDocument doc = parse_kv("# level one\n"
                                    "title = First\n"
                                    "\n"
                                    "[level]\n"
                                    "  location = arena  \n"
                                    "length=60\n");
    ASSERT_EQ(doc.sections.size(), 2u);
    EXPECT_EQ(doc.sections[0].name(), "");
    EXPECT_EQ(doc.sections[0].text("title", ""), "First");
    const auto* level = doc.first("level");
    ASSERT_NE(level, nullptr);
    EXPECT_EQ(level->line(), 4);
    EXPECT_EQ(level->require_text("location"), "arena");
    EXPECT_DOUBLE_EQ(level->require_number("length"), 60.0);
    EXPECT_EQ(level->find("length")->line, 6);
}

TEST(KvFile, RepeatedSectionsKeepOrder)
{
    const KvDocument doc = parse_kv("[hoop]\nx = 10\n[cup]\nx = 50\n[hoop]\nx = 20\n");
    const auto hoops = doc.all("hoop");
    ASSERT_EQ(hoops.size(), 2u);
    EXPECT_DOUBLE_EQ(hoops[0]->require_number("x"), 10.0);
    EXPECT_DOUBLE_EQ(hoops[1]->require_number("x"), 20.0);
    EXPECT_EQ(doc.first("ring"), nullptr);
}

TEST(KvFile, HandlesWindowsLineEndingsAndMissingFinalNewline)
{
    const KvDocument doc = parse_kv("[a]\r\nk = v\r\nn = 2");
    EXPECT_EQ(doc.first("a")->text("k", ""), "v");
    EXPECT_DOUBLE_EQ(doc.first("a")->require_number("n"), 2.0);
}

TEST(KvFile, NumbersAcceptCommaAndSign)
{
    const KvDocument doc = parse_kv("a = 1,5\nb = -0.25\nc = +3\n");
    const auto& root = doc.sections[0];
    EXPECT_DOUBLE_EQ(root.number("a", 0.0), 1.5);
    EXPECT_DOUBLE_EQ(root.number("b", 0.0), -0.25);
    EXPECT_DOUBLE_EQ(root.number("c", 0.0), 3.0);
    EXPECT_DOUBLE_EQ(root.number("missing", 7.0), 7.0);
}

TEST(KvFile, BadNumberReportsLine)
{
    const KvDocument doc = parse_kv("\nspeed = fast\n");
    try
    {
        doc.sections[0].number("speed", 1.0);
        FAIL() << "expected KvError";
    }
    catch (const KvError& error)
    {
        EXPECT_EQ(error.line(), 2);
        EXPECT_NE(std::string{error.what()}.find("speed"), std::string::npos);
    }
}

TEST(KvFile, Flags)
{
    const KvDocument doc = parse_kv("a = yes\nb = No\nc = on\nd = 0\ne = maybe\n");
    const auto& root = doc.sections[0];
    EXPECT_TRUE(root.flag("a", false));
    EXPECT_FALSE(root.flag("b", true));
    EXPECT_TRUE(root.flag("c", false));
    EXPECT_FALSE(root.flag("d", true));
    EXPECT_TRUE(root.flag("missing", true));
    EXPECT_THROW(root.flag("e", false), KvError);
}

TEST(KvFile, SyntaxErrorsReportLine)
{
    EXPECT_EQ(error_line("a = 1\njust words\n"), 2);
    EXPECT_EQ(error_line("[level\n"), 1);
    EXPECT_EQ(error_line("[]\n"), 1);
    EXPECT_EQ(error_line("[two words]\n"), 1);
    EXPECT_EQ(error_line("[circus.]\n"), 1);
    EXPECT_EQ(error_line("[.elephant]\n"), 1);
    EXPECT_EQ(error_line("[circus..elephant]\n"), 1);
    EXPECT_EQ(error_line("\n\n = value\n"), 3);
    EXPECT_EQ(error_line("[a]\nk = 1\nk = 2\n"), 3);
}

TEST(KvFile, SectionNamesMayHaveDots)
{
    const KvDocument doc = parse_kv("[circus.elephant]\nwalk_speed = 3\n");
    ASSERT_NE(doc.first("circus.elephant"), nullptr);
    EXPECT_DOUBLE_EQ(doc.first("circus.elephant")->number("walk_speed", 0.0), 3.0);
    EXPECT_EQ(error_line("[a]\nwalk.speed = 3\n"), 2);
}

TEST(KvFile, SameKeyInDifferentSectionsIsFine)
{
    EXPECT_NO_THROW(parse_kv("[hoop]\nx = 1\n[hoop]\nx = 2\n"));
}

TEST(KvFile, RequiredValuesReportSectionLine)
{
    const KvDocument doc = parse_kv("\n[hoop]\ny = 1\n");
    try
    {
        doc.first("hoop")->require_number("x");
        FAIL() << "expected KvError";
    }
    catch (const KvError& error)
    {
        EXPECT_EQ(error.line(), 2);
        EXPECT_EQ(std::string{error.what()}, "line 2: missing 'x' in [hoop]");
    }
}

TEST(KvFile, LoadFileAddsFileName)
{
    const auto path = std::filesystem::temp_directory_path() / "ce_test_kv_bad.txt";
    {
        std::ofstream out{path};
        out << "[level]\noops\n";
    }
    try
    {
        ce::core::load_kv_file(path);
        FAIL() << "expected KvError";
    }
    catch (const KvError& error)
    {
        EXPECT_EQ(std::string{error.what()}, "ce_test_kv_bad.txt, line 2: expected 'key = value'");
        EXPECT_EQ(error.file(), "ce_test_kv_bad.txt");
    }
    std::filesystem::remove(path);
    EXPECT_THROW(ce::core::load_kv_file(path), KvError);
}
