#pragma once

#include <array>
#include <cstddef>

namespace ce::core
{

// Buttons of the game, not keys. Which key is which button is decided in src/input/.
enum class GameButton
{
    TwoLegs,
    Jump,
};

inline constexpr std::size_t game_button_count = 2;

// Which buttons are down right now, as read from the keyboard or a script.
using ButtonsDown = std::array<bool, game_button_count>;

// Button state for one logic step: pressed and released are true only in the step
// where the change happened; held is true in every step while the button is down.
class InputFrame
{
public:
    bool pressed(GameButton button) const;
    bool held(GameButton button) const;
    bool released(GameButton button) const;

    void set(GameButton button, bool was_down, bool is_down);

private:
    ButtonsDown pressed_{};
    ButtonsDown held_{};
    ButtonsDown released_{};
};

// Turns "which buttons are down" in consecutive steps into InputFrames.
class InputTracker
{
public:
    InputFrame next(const ButtonsDown& down);

private:
    ButtonsDown previous_{};
};

} // namespace ce::core
