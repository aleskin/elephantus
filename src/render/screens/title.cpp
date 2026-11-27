#include "core/game_info.hpp"
#include "render/screens/screens.hpp"

#include <raylib.h>

#include <cmath>
#include <string>

namespace ce::render
{

namespace
{

constexpr Color background_top{40, 30, 70, 255};
constexpr Color background_bottom{10, 8, 20, 255};
constexpr Color title_colour{250, 204, 80, 255};
constexpr Color title_shadow{120, 40, 40, 255};
constexpr Color stripe_red{190, 40, 50, 255};
constexpr Color stripe_cream{240, 225, 190, 255};
constexpr Color elephant_grey{160, 160, 170, 255};
constexpr Color elephant_dark{120, 120, 132, 255};
constexpr Color text_colour{230, 230, 240, 255};

void draw_centred(const char* text, int y, int size, Color colour)
{
    DrawText(text, (GetScreenWidth() - MeasureText(text, size)) / 2, y, size, colour);
}

// Circus tent stripes along the top edge.
void draw_tent(int width)
{
    const int stripe = 80;
    for (int x = 0, i = 0; x < width; x += stripe, ++i)
    {
        const Color colour = i % 2 == 0 ? stripe_red : stripe_cream;
        DrawTriangle({static_cast<float>(x), 0.0F}, {static_cast<float>(x) + stripe / 2.0F, 70.0F},
                     {static_cast<float>(x + stripe), 0.0F}, colour);
    }
}

// A grey elephant from circles, looking right, standing at (x, floor).
void draw_elephant(float x, float floor, float bob)
{
    const float y = floor - 110.0F + bob;
    for (const float leg : {-60.0F, -25.0F, 25.0F, 60.0F})
    {
        DrawRectangleRounded({x + leg - 16.0F, y + 20.0F, 32.0F, floor - y - 20.0F}, 0.4F, 6,
                             elephant_dark);
    }
    DrawEllipse(static_cast<int>(x), static_cast<int>(y), 100.0F, 70.0F, elephant_grey);
    DrawCircleV({x + 105.0F, y - 40.0F}, 55.0F, elephant_grey);
    DrawEllipse(static_cast<int>(x + 75.0F), static_cast<int>(y - 35.0F), 35.0F, 50.0F,
                elephant_dark);
    DrawLineEx({x + 145.0F, y - 25.0F}, {x + 175.0F, y + 25.0F}, 26.0F, elephant_grey);
    DrawLineEx({x + 175.0F, y + 25.0F}, {x + 200.0F, y + 10.0F}, 20.0F, elephant_grey);
    DrawCircleV({x + 125.0F, y - 55.0F}, 7.0F, BLACK);
    DrawLineEx({x - 98.0F, y - 5.0F}, {x - 125.0F, y + 35.0F}, 6.0F, elephant_dark);
}

} // namespace

void draw_title(double time)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangleGradientV(0, 0, width, height, background_top, background_bottom);
    draw_tent(width);

    const std::string name{core::game_title()};
    const int size = 120;
    const int x = (width - MeasureText(name.c_str(), size)) / 2;
    DrawText(name.c_str(), x + 6, 136, size, title_shadow);
    DrawText(name.c_str(), x, 130, size, title_colour);

    const float floor = static_cast<float>(height) - 170.0F;
    DrawEllipse(width / 2, static_cast<int>(floor), 190.0F, 18.0F, Fade(BLACK, 0.4F));
    const float bob = 6.0F * static_cast<float>(std::sin(time * 3.0));
    draw_elephant(static_cast<float>(width) / 2.0F - 40.0F, floor, bob);

    if (std::fmod(time, 1.0) < 0.65)
    {
        draw_centred("Press A", height - 120, 44, text_colour);
    }
    const std::string version = "v" + std::string{core::game_version()};
    DrawText(version.c_str(), width - MeasureText(version.c_str(), 20) - 16, height - 34, 20,
             Fade(text_colour, 0.5F));
}

} // namespace ce::render
