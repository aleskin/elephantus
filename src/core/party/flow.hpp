#pragma once

#include "core/party/minigame.hpp"

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace ce::core
{

// Menu buttons pressed in this frame. Arrows, the d-pad and the left stick move; A chooses,
// B goes back (like on a Nintendo controller).
struct MenuInput
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool confirm = false;
    bool back = false;
};

// Which menu buttons are down right now; MenuTracker turns it into MenuInput.
enum class MenuButton
{
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Back,
};

inline constexpr std::size_t menu_button_count = 6;
using MenuButtonsDown = std::array<bool, menu_button_count>;

class MenuTracker
{
public:
    MenuInput next(const MenuButtonsDown& down);

private:
    MenuButtonsDown previous_{};
};

// The screens around the mini-games (SPEC.md, "Рамка игры").
enum class Screen
{
    Title,
    MainMenu,
    MinigameList,
    Minigame,
    // "Settings": which keys and gamepad buttons the game buttons are (ControlsMenu).
    Controls,
    Quit,
};

// TODO(CEL-00032): "Story" and "Credits" come with their own steps.
enum class MainMenuItem
{
    Minigames,
    Settings,
    Quit,
};

inline constexpr std::array<MainMenuItem, 3> main_menu_items = {
    MainMenuItem::Minigames, MainMenuItem::Settings, MainMenuItem::Quit};

// Moves between the screens: title -> main menu -> mini-game list -> mini-game -> back to
// the list; main menu -> settings and back. Knows nothing about drawing or keys. On the
// settings screen only "back" is handled here, the rest goes to ControlsMenu.
class PartyFlow
{
public:
    // The mini-games open for play, in menu order.
    explicit PartyFlow(std::vector<MinigameInfo> minigames);

    Screen screen() const;
    // The highlighted item of the main menu or of the mini-game list.
    std::size_t cursor() const;
    std::span<const MinigameInfo> minigames() const;
    // The mini-game being played (valid on Screen::Minigame).
    const MinigameInfo& chosen() const;

    void input(const MenuInput& input);
    // The mini-game is over, or the player left it: back to the list.
    void minigame_over();

private:
    std::size_t item_count() const;

    std::vector<MinigameInfo> minigames_;
    Screen screen_ = Screen::Title;
    std::size_t main_cursor_ = 0;
    std::size_t list_cursor_ = 0;
};

} // namespace ce::core
