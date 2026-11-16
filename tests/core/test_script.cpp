#include "core/script/script.hpp"

#include <gtest/gtest.h>

using ce::core::GameButton;
using ce::core::KvError;
using ce::core::parse_kv;
using ce::core::Script;
using ce::core::ScriptPlayer;

namespace
{

constexpr double rate = 120.0;

Script script_from(std::string_view text)
{
    return ce::core::parse_script(parse_kv(text), rate);
}

std::size_t index(GameButton button)
{
    return static_cast<std::size_t>(button);
}

} // namespace

TEST(Script, TimesBecomeStepsSortedByTime)
{
    const Script script = script_from("[shot]\nat = 1\nname = late\n"
                                      "[press]\nat = 0,5\nbutton = x\n"
                                      "[end]\nat = 2\n");
    EXPECT_EQ(script.end_step, 240);
    ASSERT_EQ(script.actions.size(), 2u);
    EXPECT_EQ(script.actions[0].step, 60);
    EXPECT_EQ(script.actions[0].kind, ce::core::ScriptAction::Kind::Press);
    EXPECT_EQ(script.actions[0].button, GameButton::X);
    EXPECT_EQ(script.actions[1].step, 120);
    EXPECT_EQ(script.actions[1].name, "late");
}

TEST(Script, Errors)
{
    EXPECT_THROW(script_from("[shot]\nat = 0\nname = a\n"), KvError);
    EXPECT_THROW(script_from("[end]\nat = 1\n[end]\nat = 2\n"), KvError);
    EXPECT_THROW(script_from("[press]\nat = 2\nbutton = x\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("[press]\nat = 0\nbutton = fly\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("[press]\nbutton = x\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("[shot]\nat = -1\nname = a\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("[shot]\nat = 0\nname = ../a\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("[wait]\nat = 0\n[end]\nat = 1\n"), KvError);
    EXPECT_THROW(script_from("at = 0\n[end]\nat = 1\n"), KvError);
}

TEST(ScriptPlayer, ButtonsAndShotsStepByStep)
{
    ScriptPlayer player{script_from("[press]\nat = 0\nbutton = a\n"
                                    "[press]\nat = 0.025\nbutton = x\n"
                                    "[release]\nat = 0.025\nbutton = a\n"
                                    "[shot]\nat = 0.025\nname = a\n"
                                    "[shot]\nat = 0.025\nname = b\n"
                                    "[end]\nat = 0.05\n")};
    int steps = 0;
    for (; !player.done(); player.next())
    {
        const auto& down = player.buttons();
        if (player.step() < 3)
        {
            EXPECT_TRUE(down[index(GameButton::A)]);
            EXPECT_FALSE(down[index(GameButton::X)]);
            EXPECT_TRUE(player.shots().empty());
        }
        else
        {
            EXPECT_FALSE(down[index(GameButton::A)]);
            EXPECT_TRUE(down[index(GameButton::X)]);
        }
        if (player.step() == 3)
        {
            EXPECT_EQ(player.shots(), (std::vector<std::string>{"a", "b"}));
        }
        ++steps;
    }
    EXPECT_EQ(steps, 7);
}

TEST(Script, ButtonNames)
{
    EXPECT_EQ(ce::core::button_from_name("x"), GameButton::X);
    EXPECT_EQ(ce::core::button_from_name("a"), GameButton::A);
    EXPECT_FALSE(ce::core::button_from_name("A").has_value());
}

TEST(Script, BundledScriptsLoad)
{
    const auto folder = std::filesystem::path{CE_SCRIPTS_DIR};
    int count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator{folder})
    {
        if (entry.path().extension() == ".txt")
        {
            SCOPED_TRACE(entry.path().string());
            EXPECT_NO_THROW(ce::core::load_script(entry.path(), rate));
            ++count;
        }
    }
    EXPECT_GT(count, 0);
}
