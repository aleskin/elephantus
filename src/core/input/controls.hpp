#pragma once

#include "core/input/input_frame.hpp"
#include "core/input/pad_layout.hpp"
#include "core/text/kv_file.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace ce::core
{

// Keyboard keys are numbered like in raylib and GLFW: letters and digits by their
// ASCII code, the rest from 256 up. Only the keys the game needs by name are here.
namespace keys
{
inline constexpr int a = 65;
inline constexpr int y = 89;
inline constexpr int escape = 256;
inline constexpr int right = 262;
inline constexpr int left = 263;
inline constexpr int down = 264;
inline constexpr int up = 265;
} // namespace keys

// Gamepad buttons by their place, not by the letters printed on them.
enum class PadButton
{
    FaceRight,
    FaceDown,
    FaceUp,
    FaceLeft,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    L1,
    L2,
    R1,
    R2,
    Select,
    LeftStick,
    RightStick,
};

inline constexpr std::size_t pad_button_count = 15;

// Which keyboard key and which gamepad button press one game button.
struct Binding
{
    int key = 0;
    PadButton pad = PadButton::FaceRight;

    bool operator==(const Binding&) const = default;
};

// The controls of the game, changed in "Settings" -> "Controls" and kept in a file of the
// user (SPEC.md, "Кнопки").
struct Controls
{
    std::array<Binding, game_button_count> buttons{};

    const Binding& operator[](GameButton button) const;
    Binding& operator[](GameButton button);

    bool operator==(const Controls&) const = default;
};

// Keyboard: A walks, Left is X, Right is B, Y stays Y. Gamepad: walking is the d-pad
// "forward" (right), X, B and Y are the face buttons with these letters for the layout.
Controls default_controls(PadLayout layout);

// What a game button does, for the menu and the hints: "Идти", "Прыжок 1", ...
std::string_view action_name(GameButton button);

// Names in the controls file: "a", "left", "space", "key_123" for keys without a name;
// "dpad_right", "face_down", "l1", ... for gamepad buttons.
std::string key_name(int key);
std::optional<int> key_from_name(std::string_view name);
std::string_view pad_button_name(PadButton button);
std::optional<PadButton> pad_button_from_name(std::string_view name);

// What the player reads on the screen: "A", "Влево", "Пробел"; face buttons by their
// letter for the layout, "Крестовина вправо", "L1".
std::string key_label(int key);
std::string pad_button_label(PadButton button, PadLayout layout);

// Controls file: one section per game button, [a] keyboard = a, gamepad = dpad_right.
// Missing sections or keys keep the defaults; unknown names are an error (KvError).
Controls parse_controls(const KvDocument& document, PadLayout layout);
std::string format_controls(const Controls& controls);
// nullopt when there is no file yet; KvError with the file name when it is broken.
std::optional<Controls> load_controls(const std::filesystem::path& path, PadLayout layout);

} // namespace ce::core
