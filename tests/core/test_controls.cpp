#include "core/input/controls.hpp"
#include "core/settings/user_files.hpp"
#include "core/text/kv_file.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <optional>

using ce::core::action_name;
using ce::core::ButtonLabels;
using ce::core::control_labels;
using ce::core::Controls;
using ce::core::default_controls;
using ce::core::format_controls;
using ce::core::GameButton;
using ce::core::key_from_name;
using ce::core::key_label;
using ce::core::key_name;
using ce::core::KvError;
using ce::core::load_controls;
using ce::core::pad_button_from_name;
using ce::core::pad_button_label;
using ce::core::pad_button_name;
using ce::core::PadButton;
using ce::core::PadLayout;
using ce::core::parse_controls;
using ce::core::parse_kv;
using ce::core::write_text_file;
namespace keys = ce::core::keys;

TEST(Controls, KeyboardDefaultsFromTheOwner)
{
    const Controls controls = default_controls(PadLayout::Nintendo);
    EXPECT_EQ(controls[GameButton::A].key, keys::a);
    EXPECT_EQ(controls[GameButton::X].key, keys::left);
    EXPECT_EQ(controls[GameButton::B].key, keys::right);
    EXPECT_EQ(controls[GameButton::Y].key, keys::y);
}

TEST(Controls, GamepadWalksWithTheDpadAndJumpsWithXThenB)
{
    const Controls nintendo = default_controls(PadLayout::Nintendo);
    EXPECT_EQ(nintendo[GameButton::A].pad, PadButton::DpadRight);
    EXPECT_EQ(nintendo[GameButton::X].pad, PadButton::FaceUp);
    EXPECT_EQ(nintendo[GameButton::B].pad, PadButton::FaceDown);
    EXPECT_EQ(nintendo[GameButton::Y].pad, PadButton::FaceLeft);

    const Controls xbox = default_controls(PadLayout::Xbox);
    EXPECT_EQ(xbox[GameButton::A].pad, PadButton::DpadRight);
    EXPECT_EQ(xbox[GameButton::X].pad, PadButton::FaceLeft);
    EXPECT_EQ(xbox[GameButton::B].pad, PadButton::FaceRight);
}

TEST(Controls, ActionsHaveNames)
{
    EXPECT_EQ(action_name(GameButton::A), "Идти");
    EXPECT_EQ(action_name(GameButton::X), "Прыжок 1");
    EXPECT_EQ(action_name(GameButton::B), "Прыжок 2");
}

TEST(Controls, KeyNamesGoBothWays)
{
    for (const int key : {keys::a, keys::left, keys::right, keys::up, int{'Z'}, int{'7'}, 32, 290,
                          301, 320, 340, 999})
    {
        EXPECT_EQ(key_from_name(key_name(key)), key) << key_name(key);
    }
    EXPECT_EQ(key_name(keys::left), "left");
    EXPECT_EQ(key_name(290), "f1");
    EXPECT_EQ(key_name(999), "key_999");
    EXPECT_EQ(key_from_name("nothing"), std::nullopt);
    EXPECT_EQ(key_from_name("f13"), std::nullopt);
}

TEST(Controls, PadButtonNamesGoBothWays)
{
    for (std::size_t i = 0; i < ce::core::pad_button_count; ++i)
    {
        const auto button = static_cast<PadButton>(i);
        EXPECT_EQ(pad_button_from_name(pad_button_name(button)), button);
    }
    EXPECT_EQ(pad_button_from_name("start"), std::nullopt);
}

TEST(Controls, LabelsForTheScreen)
{
    EXPECT_EQ(key_label(keys::a), "A");
    EXPECT_EQ(key_label(keys::left), "Влево");
    EXPECT_EQ(key_label(32), "Пробел");
    EXPECT_EQ(pad_button_label(PadButton::FaceUp, PadLayout::Nintendo), "X");
    EXPECT_EQ(pad_button_label(PadButton::FaceUp, PadLayout::Xbox), "Y");
    EXPECT_EQ(pad_button_label(PadButton::DpadRight, PadLayout::Nintendo), "Крестовина вправо");
}

TEST(Controls, FileRoundTrip)
{
    Controls controls = default_controls(PadLayout::Nintendo);
    controls[GameButton::A].key = 32;
    controls[GameButton::B].pad = PadButton::R1;
    const Controls read = parse_controls(parse_kv(format_controls(controls)), PadLayout::Xbox);
    for (std::size_t i = 0; i < ce::core::game_button_count; ++i)
    {
        EXPECT_EQ(read.buttons[i].key, controls.buttons[i].key);
        EXPECT_EQ(read.buttons[i].pad, controls.buttons[i].pad);
    }
}

TEST(Controls, MissingEntriesKeepDefaults)
{
    const Controls read = parse_controls(parse_kv("[x]\nkeyboard = z\n"), PadLayout::Nintendo);
    const Controls defaults = default_controls(PadLayout::Nintendo);
    EXPECT_EQ(read[GameButton::X].key, 'Z');
    EXPECT_EQ(read[GameButton::X].pad, defaults[GameButton::X].pad);
    EXPECT_EQ(read[GameButton::A].key, defaults[GameButton::A].key);
}

TEST(Controls, MistakesAreErrors)
{
    EXPECT_THROW(parse_controls(parse_kv("[q]\nkeyboard = a\n"), PadLayout::Nintendo), KvError);
    EXPECT_THROW(parse_controls(parse_kv("[a]\nkeyboard = nothing\n"), PadLayout::Nintendo),
                 KvError);
    EXPECT_THROW(parse_controls(parse_kv("[a]\ngamepad = start\n"), PadLayout::Nintendo), KvError);
    EXPECT_THROW(parse_controls(parse_kv("[a]\nmouse = left\n"), PadLayout::Nintendo), KvError);
    EXPECT_THROW(parse_controls(parse_kv("keyboard = a\n"), PadLayout::Nintendo), KvError);
}

TEST(UserFiles, WriteCreatesFoldersAndReplacesTheFile)
{
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / "elephantus_test_user_files";
    std::filesystem::remove_all(dir);
    const std::filesystem::path file = dir / "deep" / "controls.txt";
    write_text_file(file, "[a]\nkeyboard = q\n");
    write_text_file(file, "[a]\nkeyboard = w\n");
    const std::optional<Controls> read = load_controls(file, PadLayout::Nintendo);
    ASSERT_TRUE(read);
    EXPECT_EQ((*read)[GameButton::A].key, 'W');
    EXPECT_EQ(load_controls(dir / "none.txt", PadLayout::Nintendo), std::nullopt);
    write_text_file(file, "[a]\nkeyboard = ???\n");
    EXPECT_THROW(load_controls(file, PadLayout::Nintendo), KvError);
    EXPECT_FALSE(std::filesystem::exists(file.string() + ".tmp"));
    std::filesystem::remove_all(dir);
}

TEST(Controls, LabelsFollowTheBindingsAndTheDevice)
{
    Controls controls = default_controls(PadLayout::Nintendo);
    const ButtonLabels keyboard = control_labels(controls, false, PadLayout::Nintendo);
    EXPECT_EQ(keyboard[0], "A");
    EXPECT_EQ(keyboard[2], "Влево");
    EXPECT_EQ(keyboard[1], "Вправо");
    const ButtonLabels pad = control_labels(controls, true, PadLayout::Nintendo);
    EXPECT_EQ(pad[0], "Крестовина вправо");
    EXPECT_EQ(pad[2], "X");
    EXPECT_EQ(pad[1], "B");
    controls[GameButton::A].key = keys::up;
    EXPECT_EQ(control_labels(controls, false, PadLayout::Nintendo)[0], "Вверх");
}
