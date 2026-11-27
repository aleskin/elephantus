#include "core/input/controls.hpp"

#include <charconv>

namespace ce::core
{

namespace
{

struct NamedKey
{
    int key;
    std::string_view name;
    std::string_view label;
};

constexpr std::array<NamedKey, 27> named_keys = {{
    {32, "space", "Пробел"},
    {39, "apostrophe", "'"},
    {44, "comma", ","},
    {45, "minus", "-"},
    {46, "period", "."},
    {47, "slash", "/"},
    {59, "semicolon", ";"},
    {61, "equal", "="},
    {91, "left_bracket", "["},
    {92, "backslash", "\\"},
    {93, "right_bracket", "]"},
    {96, "grave", "`"},
    {257, "enter", "Enter"},
    {258, "tab", "Tab"},
    {259, "backspace", "Backspace"},
    {260, "insert", "Insert"},
    {261, "delete", "Delete"},
    {keys::right, "right", "Вправо"},
    {keys::left, "left", "Влево"},
    {keys::down, "down", "Вниз"},
    {keys::up, "up", "Вверх"},
    {340, "left_shift", "Левый Shift"},
    {341, "left_ctrl", "Левый Ctrl"},
    {342, "left_alt", "Левый Alt"},
    {344, "right_shift", "Правый Shift"},
    {345, "right_ctrl", "Правый Ctrl"},
    {346, "right_alt", "Правый Alt"},
}};

constexpr int f1 = 290;
constexpr int f12 = 301;
constexpr int keypad_0 = 320;
constexpr int keypad_9 = 329;

struct NamedPadButton
{
    PadButton button;
    std::string_view name;
    std::string_view label;
};

constexpr std::array<NamedPadButton, pad_button_count> pad_buttons = {{
    {PadButton::FaceRight, "face_right", ""},
    {PadButton::FaceDown, "face_down", ""},
    {PadButton::FaceUp, "face_up", ""},
    {PadButton::FaceLeft, "face_left", ""},
    {PadButton::DpadUp, "dpad_up", "Крестовина вверх"},
    {PadButton::DpadDown, "dpad_down", "Крестовина вниз"},
    {PadButton::DpadLeft, "dpad_left", "Крестовина влево"},
    {PadButton::DpadRight, "dpad_right", "Крестовина вправо"},
    {PadButton::L1, "l1", "L1"},
    {PadButton::L2, "l2", "L2"},
    {PadButton::R1, "r1", "R1"},
    {PadButton::R2, "r2", "R2"},
    {PadButton::Select, "select", "Select"},
    {PadButton::LeftStick, "left_stick", "Левый стик"},
    {PadButton::RightStick, "right_stick", "Правый стик"},
}};

constexpr std::array<std::string_view, game_button_count> button_names = {"a", "b", "x", "y"};
constexpr std::array<std::string_view, game_button_count> button_labels = {"A", "B", "X", "Y"};

std::size_t index(GameButton button)
{
    return static_cast<std::size_t>(button);
}

bool is_letter(int key)
{
    return key >= 'A' && key <= 'Z';
}

bool is_digit(int key)
{
    return key >= '0' && key <= '9';
}

std::optional<int> parse_int(std::string_view text)
{
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size())
    {
        return std::nullopt;
    }
    return value;
}

PadButton face_button(PadLayout layout, GameButton button)
{
    for (const PadFace face : {PadFace::Right, PadFace::Down, PadFace::Up, PadFace::Left})
    {
        if (button_at(layout, face) == button)
        {
            switch (face)
            {
            case PadFace::Right:
                return PadButton::FaceRight;
            case PadFace::Down:
                return PadButton::FaceDown;
            case PadFace::Up:
                return PadButton::FaceUp;
            case PadFace::Left:
                return PadButton::FaceLeft;
            }
        }
    }
    return PadButton::FaceRight;
}

std::optional<PadFace> face_of(PadButton button)
{
    switch (button)
    {
    case PadButton::FaceRight:
        return PadFace::Right;
    case PadButton::FaceDown:
        return PadFace::Down;
    case PadButton::FaceUp:
        return PadFace::Up;
    case PadButton::FaceLeft:
        return PadFace::Left;
    default:
        return std::nullopt;
    }
}

} // namespace

const Binding& Controls::operator[](GameButton button) const
{
    return buttons[index(button)];
}

Binding& Controls::operator[](GameButton button)
{
    return buttons[index(button)];
}

Controls default_controls(PadLayout layout)
{
    Controls controls;
    controls[GameButton::A] = Binding{keys::a, PadButton::DpadRight};
    controls[GameButton::X] = Binding{keys::left, face_button(layout, GameButton::X)};
    controls[GameButton::B] = Binding{keys::right, face_button(layout, GameButton::B)};
    controls[GameButton::Y] = Binding{keys::y, face_button(layout, GameButton::Y)};
    return controls;
}

std::string_view action_name(GameButton button)
{
    switch (button)
    {
    case GameButton::A:
        return "Идти";
    case GameButton::X:
        return "Прыжок 1";
    case GameButton::B:
        return "Прыжок 2";
    case GameButton::Y:
        // TODO(CEL-00011): the super jump will need it.
        return "Кнопка Y (пока не нужна)";
    }
    return "";
}

std::string key_name(int key)
{
    if (is_letter(key))
    {
        return std::string(1, static_cast<char>(key - 'A' + 'a'));
    }
    if (is_digit(key))
    {
        return std::string(1, static_cast<char>(key));
    }
    if (key >= f1 && key <= f12)
    {
        return "f" + std::to_string(key - f1 + 1);
    }
    if (key >= keypad_0 && key <= keypad_9)
    {
        return "kp_" + std::to_string(key - keypad_0);
    }
    for (const NamedKey& named : named_keys)
    {
        if (named.key == key)
        {
            return std::string{named.name};
        }
    }
    return "key_" + std::to_string(key);
}

std::optional<int> key_from_name(std::string_view name)
{
    if (name.size() == 1 && name[0] >= 'a' && name[0] <= 'z')
    {
        return name[0] - 'a' + 'A';
    }
    if (name.size() == 1 && is_digit(name[0]))
    {
        return name[0];
    }
    for (const NamedKey& named : named_keys)
    {
        if (named.name == name)
        {
            return named.key;
        }
    }
    const auto numbered = [name](std::string_view prefix, int first, int last,
                                 int offset) -> std::optional<int>
    {
        if (!name.starts_with(prefix))
        {
            return std::nullopt;
        }
        const std::optional<int> number = parse_int(name.substr(prefix.size()));
        if (!number || *number + offset < first || *number + offset > last)
        {
            return std::nullopt;
        }
        return *number + offset;
    };
    if (const std::optional<int> key = numbered("kp_", keypad_0, keypad_9, keypad_0))
    {
        return key;
    }
    if (const std::optional<int> key = numbered("key_", 1, 999, 0))
    {
        return key;
    }
    if (const std::optional<int> key = numbered("f", f1, f12, f1 - 1))
    {
        return key;
    }
    return std::nullopt;
}

std::string_view pad_button_name(PadButton button)
{
    return pad_buttons[static_cast<std::size_t>(button)].name;
}

std::optional<PadButton> pad_button_from_name(std::string_view name)
{
    for (const NamedPadButton& named : pad_buttons)
    {
        if (named.name == name)
        {
            return named.button;
        }
    }
    return std::nullopt;
}

std::string key_label(int key)
{
    if (is_letter(key) || is_digit(key))
    {
        return std::string(1, static_cast<char>(key));
    }
    if (key >= f1 && key <= f12)
    {
        return "F" + std::to_string(key - f1 + 1);
    }
    if (key >= keypad_0 && key <= keypad_9)
    {
        return "Num " + std::to_string(key - keypad_0);
    }
    for (const NamedKey& named : named_keys)
    {
        if (named.key == key)
        {
            return std::string{named.label};
        }
    }
    return "Клавиша " + std::to_string(key);
}

std::string pad_button_label(PadButton button, PadLayout layout)
{
    if (const std::optional<PadFace> face = face_of(button))
    {
        return std::string{button_labels[index(button_at(layout, *face))]};
    }
    return std::string{pad_buttons[static_cast<std::size_t>(button)].label};
}

Controls parse_controls(const KvDocument& document, PadLayout layout)
{
    Controls controls = default_controls(layout);
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            if (!section.entries().empty())
            {
                throw KvError(section.entries().front().line, "put keys under a section, e.g. [a]");
            }
            continue;
        }
        const std::optional<GameButton> button = button_from_name(section.name());
        if (!button)
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
        for (const KvEntry& entry : section.entries())
        {
            if (entry.key == "keyboard")
            {
                const std::optional<int> key = key_from_name(entry.value);
                if (!key)
                {
                    throw KvError(entry.line, "unknown key '" + entry.value + "'");
                }
                controls[*button].key = *key;
            }
            else if (entry.key == "gamepad")
            {
                const std::optional<PadButton> pad = pad_button_from_name(entry.value);
                if (!pad)
                {
                    throw KvError(entry.line, "unknown gamepad button '" + entry.value + "'");
                }
                controls[*button].pad = *pad;
            }
            else
            {
                throw KvError(entry.line, "unknown key '" + entry.key + "'");
            }
        }
    }
    return controls;
}

std::string format_controls(const Controls& controls)
{
    std::string text = "# Elephantus: кнопки игры. Меняются в меню «Настройки» → «Управление».\n";
    for (std::size_t i = 0; i < game_button_count; ++i)
    {
        const auto button = static_cast<GameButton>(i);
        text += "\n# " + std::string{action_name(button)} + "\n";
        text += "[" + std::string{button_names[i]} + "]\n";
        text += "keyboard = " + key_name(controls[button].key) + "\n";
        text += "gamepad = " + std::string{pad_button_name(controls[button].pad)} + "\n";
    }
    return text;
}

std::optional<Controls> load_controls(const std::filesystem::path& path, PadLayout layout)
{
    if (!std::filesystem::exists(path))
    {
        return std::nullopt;
    }
    try
    {
        return parse_controls(load_kv_file(path), layout);
    }
    catch (const KvError& error)
    {
        if (!error.file().empty())
        {
            throw;
        }
        throw KvError(error.line(), error.detail(), path.filename().string());
    }
}

} // namespace ce::core
