#pragma once

#include "core/input/controls.hpp"
#include "core/party/flow.hpp"

#include <array>
#include <cstddef>
#include <optional>

namespace ce::core
{

// The rows of the controls table in the order the elephant uses them: walk, jump 1,
// jump 2, then Y. The last row "reset" is not a button.
inline constexpr std::array<GameButton, game_button_count> controls_menu_rows = {
    GameButton::A, GameButton::X, GameButton::B, GameButton::Y};
inline constexpr std::size_t controls_menu_row_count = game_button_count + 1;

enum class ControlsColumn
{
    Keyboard,
    Gamepad,
};

// "Settings" -> "Controls": a table of actions and their keys and gamepad buttons.
// Up and down choose the row, left and right the column; A on a cell waits for the new
// key or button, A on "reset" brings back the defaults. A key that another action already
// has is swapped with it, so every action keeps a key. Escape is not given away: it leaves
// the mini-game and cancels the waiting.
class ControlsMenu
{
public:
    ControlsMenu(Controls controls, PadLayout layout);

    const Controls& controls() const;
    std::size_t row() const;
    ControlsColumn column() const;
    // nullopt on the "reset" row.
    std::optional<GameButton> button() const;
    bool on_reset() const;
    // Waiting for the new key or gamepad button of the highlighted cell.
    bool waiting() const;

    // Menu buttons while not waiting; "back" is for PartyFlow.
    void input(const MenuInput& input);
    void assign_key(int key);
    void assign_pad(PadButton pad);
    void cancel();

    // True once after the controls changed: the caller saves them.
    bool take_changed();

private:
    Controls controls_;
    PadLayout layout_;
    std::size_t row_ = 0;
    ControlsColumn column_ = ControlsColumn::Keyboard;
    bool waiting_ = false;
    bool changed_ = false;
};

} // namespace ce::core
