#include "render/screens/screens.hpp"
#include "render/text.hpp"

#include <raylib.h>

#include <string>

namespace ce::render
{

namespace
{

constexpr Color background_top{30, 34, 60, 255};
constexpr Color background_bottom{12, 12, 24, 255};
constexpr Color heading_colour{250, 204, 80, 255};
constexpr Color column_colour{150, 150, 170, 255};
constexpr Color item_colour{200, 200, 215, 255};
constexpr Color cell_colour{50, 56, 90, 255};
constexpr Color chosen_colour{20, 20, 30, 255};
constexpr Color chosen_box{250, 204, 80, 255};
constexpr Color hint_colour{150, 150, 170, 255};
constexpr Color shade{0, 0, 0, 170};
constexpr Color dialog_box{40, 46, 80, 255};

constexpr int action_x = 110;
constexpr int keyboard_x = 500;
constexpr int gamepad_x = 840;
constexpr int cell_width = 320;
constexpr int first_row_y = 210;
constexpr int row_step = 76;
constexpr int text_size = 36;
constexpr int cell_height = 56;

void draw_cell(const std::string& text, int x, int y, bool chosen)
{
    const Rectangle box{static_cast<float>(x), static_cast<float>(y - 10),
                        static_cast<float>(cell_width), static_cast<float>(cell_height)};
    DrawRectangleRounded(box, 0.4F, 8, chosen ? chosen_box : cell_colour);
    const int text_x = x + (cell_width - measure_text(text, text_size)) / 2;
    draw_text(text, text_x, y, text_size, chosen ? chosen_colour : item_colour);
}

void draw_waiting(const core::ControlsMenu& menu)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangle(0, 0, width, height, shade);
    const int box_width = 860;
    const int box_height = 220;
    const Rectangle box{static_cast<float>((width - box_width) / 2),
                        static_cast<float>((height - box_height) / 2),
                        static_cast<float>(box_width), static_cast<float>(box_height)};
    DrawRectangleRounded(box, 0.2F, 8, dialog_box);
    const std::string action{core::action_name(*menu.button())};
    const bool keyboard = menu.column() == core::ControlsColumn::Keyboard;
    const std::string ask = keyboard ? "Нажми клавишу для «" + action + "»"
                                     : "Нажми кнопку джойстика для «" + action + "»";
    const int top = static_cast<int>(box.y);
    draw_text_centred(ask, top + 50, 40, heading_colour);
    draw_text_centred(keyboard ? "Esc — отмена" : "Esc или Start — отмена", top + 130, 30,
                      item_colour);
}

} // namespace

void draw_controls(const core::ControlsMenu& menu, core::PadLayout layout)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangleGradientV(0, 0, width, height, background_top, background_bottom);

    draw_text_centred("Управление", 50, 64, heading_colour);
    const int header_y = first_row_y - 60;
    draw_text("Клавиатура", keyboard_x + (cell_width - measure_text("Клавиатура", 30)) / 2,
              header_y, 30, column_colour);
    draw_text("Джойстик", gamepad_x + (cell_width - measure_text("Джойстик", 30)) / 2, header_y, 30,
              column_colour);

    const core::Controls& controls = menu.controls();
    int y = first_row_y;
    for (std::size_t row = 0; row < core::controls_menu_rows.size(); ++row)
    {
        const core::GameButton button = core::controls_menu_rows[row];
        const bool chosen_row = menu.row() == row;
        draw_text(core::action_name(button), action_x, y, text_size,
                  chosen_row ? heading_colour : item_colour);
        draw_cell(core::key_label(controls[button].key), keyboard_x, y,
                  chosen_row && menu.column() == core::ControlsColumn::Keyboard);
        draw_cell(core::pad_button_label(controls[button].pad, layout), gamepad_x, y,
                  chosen_row && menu.column() == core::ControlsColumn::Gamepad);
        y += row_step;
    }

    y += 10;
    const std::string reset = "Сбросить всё";
    const int reset_width = 420;
    if (menu.on_reset())
    {
        const Rectangle box{static_cast<float>((width - reset_width) / 2),
                            static_cast<float>(y - 10), static_cast<float>(reset_width),
                            static_cast<float>(cell_height)};
        DrawRectangleRounded(box, 0.4F, 8, chosen_box);
    }
    draw_text_centred(reset, y, text_size, menu.on_reset() ? chosen_colour : item_colour);

    draw_text_centred("Стрелки — выбрать     A — поменять     B — назад", height - 70, 26,
                      hint_colour);

    if (menu.waiting())
    {
        draw_waiting(menu);
    }
}

} // namespace ce::render
