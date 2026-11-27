#include "core/party/flow.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

using ce::core::main_menu_items;
using ce::core::MainMenuItem;
using ce::core::MenuButton;
using ce::core::MenuButtonsDown;
using ce::core::MenuInput;
using ce::core::MenuTracker;
using ce::core::MinigameInfo;
using ce::core::PartyFlow;
using ce::core::Screen;

namespace
{

std::vector<MinigameInfo> two_games()
{
    return {MinigameInfo{"circus", "Circus Elephant"}, MinigameInfo{"other", "Other"}};
}

MenuInput up()
{
    MenuInput input;
    input.up = true;
    return input;
}

MenuInput down()
{
    MenuInput input;
    input.down = true;
    return input;
}

MenuInput confirm()
{
    MenuInput input;
    input.confirm = true;
    return input;
}

MenuInput back()
{
    MenuInput input;
    input.back = true;
    return input;
}

} // namespace

TEST(PartyFlow, NeedsAtLeastOneMinigame)
{
    EXPECT_THROW(PartyFlow{{}}, std::invalid_argument);
}

TEST(PartyFlow, StartsOnTitleAndAGoesToMainMenu)
{
    PartyFlow flow{two_games()};
    EXPECT_EQ(flow.screen(), Screen::Title);
    flow.input(back());
    EXPECT_EQ(flow.screen(), Screen::Title);
    flow.input(confirm());
    EXPECT_EQ(flow.screen(), Screen::MainMenu);
    EXPECT_EQ(main_menu_items[flow.cursor()], MainMenuItem::Minigames);
}

TEST(PartyFlow, MainMenuToListAndBack)
{
    PartyFlow flow{two_games()};
    flow.input(confirm());
    flow.input(confirm());
    EXPECT_EQ(flow.screen(), Screen::MinigameList);
    flow.input(back());
    EXPECT_EQ(flow.screen(), Screen::MainMenu);
    flow.input(back());
    EXPECT_EQ(flow.screen(), Screen::Title);
}

TEST(PartyFlow, QuitFromMainMenu)
{
    PartyFlow flow{two_games()};
    flow.input(confirm());
    flow.input(up());
    EXPECT_EQ(main_menu_items[flow.cursor()], MainMenuItem::Quit);
    flow.input(confirm());
    EXPECT_EQ(flow.screen(), Screen::Quit);
}

TEST(PartyFlow, SettingsAndBack)
{
    PartyFlow flow{two_games()};
    flow.input(confirm());
    flow.input(down());
    EXPECT_EQ(main_menu_items[flow.cursor()], MainMenuItem::Settings);
    flow.input(confirm());
    EXPECT_EQ(flow.screen(), Screen::Controls);
    flow.input(confirm());
    flow.input(down());
    EXPECT_EQ(flow.screen(), Screen::Controls);
    flow.input(back());
    EXPECT_EQ(flow.screen(), Screen::MainMenu);
    EXPECT_EQ(main_menu_items[flow.cursor()], MainMenuItem::Settings);
}

TEST(PartyFlow, CursorWrapsAround)
{
    PartyFlow flow{two_games()};
    flow.input(confirm());
    flow.input(confirm());
    EXPECT_EQ(flow.cursor(), 0U);
    flow.input(up());
    EXPECT_EQ(flow.cursor(), 1U);
    flow.input(down());
    EXPECT_EQ(flow.cursor(), 0U);
}

TEST(PartyFlow, ChooseMinigamePlayAndComeBackToList)
{
    PartyFlow flow{two_games()};
    flow.input(confirm());
    flow.input(confirm());
    flow.input(down());
    flow.input(confirm());
    EXPECT_EQ(flow.screen(), Screen::Minigame);
    EXPECT_EQ(flow.chosen().id, "other");
    flow.input(back());
    EXPECT_EQ(flow.screen(), Screen::Minigame);
    flow.minigame_over();
    EXPECT_EQ(flow.screen(), Screen::MinigameList);
    EXPECT_EQ(flow.cursor(), 1U);
}

TEST(PartyFlow, MinigameOverOutsideAMinigameChangesNothing)
{
    PartyFlow flow{two_games()};
    flow.minigame_over();
    EXPECT_EQ(flow.screen(), Screen::Title);
}

TEST(MenuTracker, PressedOnlyInTheFirstFrame)
{
    MenuTracker tracker;
    MenuButtonsDown down{};
    down[static_cast<std::size_t>(MenuButton::Confirm)] = true;
    EXPECT_TRUE(tracker.next(down).confirm);
    EXPECT_FALSE(tracker.next(down).confirm);
    down = {};
    EXPECT_FALSE(tracker.next(down).confirm);
    down[static_cast<std::size_t>(MenuButton::Up)] = true;
    const MenuInput input = tracker.next(down);
    EXPECT_TRUE(input.up);
    EXPECT_FALSE(input.down);
}
