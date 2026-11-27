#include "render/screens/menu.hpp"

#include <raylib.h>

namespace ce::render
{

namespace
{

constexpr Color background_top{30, 34, 60, 255};
constexpr Color background_bottom{12, 12, 24, 255};
constexpr Color heading_colour{250, 204, 80, 255};
constexpr Color item_colour{200, 200, 215, 255};
constexpr Color chosen_colour{20, 20, 30, 255};
constexpr Color chosen_box{250, 204, 80, 255};
constexpr Color hint_colour{150, 150, 170, 255};

void draw_centred(const std::string& text, int y, int size, Color colour)
{
    const int width = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), (GetScreenWidth() - width) / 2, y, size, colour);
}

} // namespace

void draw_menu(const std::string& heading, const std::vector<std::string>& items,
               std::size_t cursor, const std::string& hint)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangleGradientV(0, 0, width, height, background_top, background_bottom);

    draw_centred(heading, 80, 64, heading_colour);

    const int item_size = 44;
    const int item_step = 80;
    const int box_width = 560;
    int y = 230;
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        if (i == cursor)
        {
            const Rectangle box{static_cast<float>((width - box_width) / 2),
                                static_cast<float>(y - 14), static_cast<float>(box_width),
                                static_cast<float>(item_size + 28)};
            DrawRectangleRounded(box, 0.4F, 8, chosen_box);
            draw_centred(items[i], y, item_size, chosen_colour);
        }
        else
        {
            draw_centred(items[i], y, item_size, item_colour);
        }
        y += item_step;
    }

    draw_centred(hint, height - 70, 26, hint_colour);
}

} // namespace ce::render
