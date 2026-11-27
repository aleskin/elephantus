#include "core/party/flow.hpp"

#include <stdexcept>
#include <utility>

namespace ce::core
{

MenuInput MenuTracker::next(const MenuButtonsDown& down)
{
    const auto pressed = [&](MenuButton button)
    {
        const auto i = static_cast<std::size_t>(button);
        return down[i] && !previous_[i];
    };
    const MenuInput input{pressed(MenuButton::Up),      pressed(MenuButton::Down),
                          pressed(MenuButton::Left),    pressed(MenuButton::Right),
                          pressed(MenuButton::Confirm), pressed(MenuButton::Back)};
    previous_ = down;
    return input;
}

PartyFlow::PartyFlow(std::vector<MinigameInfo> minigames)
    : minigames_(std::move(minigames))
{
    if (minigames_.empty())
    {
        throw std::invalid_argument("no mini-games to play");
    }
}

Screen PartyFlow::screen() const
{
    return screen_;
}

std::size_t PartyFlow::cursor() const
{
    return screen_ == Screen::MainMenu ? main_cursor_ : list_cursor_;
}

std::span<const MinigameInfo> PartyFlow::minigames() const
{
    return minigames_;
}

const MinigameInfo& PartyFlow::chosen() const
{
    return minigames_[list_cursor_];
}

std::size_t PartyFlow::item_count() const
{
    return screen_ == Screen::MainMenu ? main_menu_items.size() : minigames_.size();
}

void PartyFlow::input(const MenuInput& input)
{
    if (screen_ == Screen::MainMenu || screen_ == Screen::MinigameList)
    {
        std::size_t& cursor = screen_ == Screen::MainMenu ? main_cursor_ : list_cursor_;
        const std::size_t count = item_count();
        if (input.up)
        {
            cursor = (cursor + count - 1) % count;
        }
        if (input.down)
        {
            cursor = (cursor + 1) % count;
        }
    }

    switch (screen_)
    {
    case Screen::Title:
        if (input.confirm)
        {
            screen_ = Screen::MainMenu;
        }
        break;
    case Screen::MainMenu:
        if (input.confirm)
        {
            switch (main_menu_items[main_cursor_])
            {
            case MainMenuItem::Minigames:
                screen_ = Screen::MinigameList;
                break;
            case MainMenuItem::Settings:
                screen_ = Screen::Controls;
                break;
            case MainMenuItem::Quit:
                screen_ = Screen::Quit;
                break;
            }
        }
        else if (input.back)
        {
            screen_ = Screen::Title;
        }
        break;
    case Screen::MinigameList:
        if (input.confirm)
        {
            screen_ = Screen::Minigame;
        }
        else if (input.back)
        {
            screen_ = Screen::MainMenu;
        }
        break;
    case Screen::Controls:
        if (input.back)
        {
            screen_ = Screen::MainMenu;
        }
        break;
    case Screen::Minigame:
    case Screen::Quit:
        break;
    }
}

void PartyFlow::minigame_over()
{
    if (screen_ == Screen::Minigame)
    {
        screen_ = Screen::MinigameList;
    }
}

} // namespace ce::core
