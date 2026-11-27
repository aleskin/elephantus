#include "render/minigames/circus/hud.hpp"

#include <raylib.h>

#include <string>

namespace ce::render::circus
{

namespace
{

constexpr Color panel{0, 0, 0, 150};
constexpr Color text_colour{235, 235, 245, 255};
constexpr Color button_colour{250, 204, 80, 255};
constexpr Color good_colour{120, 230, 120, 255};
constexpr Color bad_colour{255, 110, 100, 255};

// A round game button with its letter; returns the x right after it.
int draw_button(char letter, int x, int y)
{
    const int radius = 18;
    DrawCircle(x + radius, y + radius, static_cast<float>(radius), button_colour);
    const char text[2] = {letter, '\0'};
    const int size = 24;
    DrawText(text, x + radius - MeasureText(text, size) / 2, y + radius - size / 2 + 1, size,
             BLACK);
    return x + 2 * radius + 8;
}

int draw_words(const char* words, int x, int y)
{
    const int size = 24;
    DrawText(words, x, y + 7, size, text_colour);
    return x + MeasureText(words, size) + 10;
}

void draw_big_centred(const char* text, int y, Color colour)
{
    const int size = 64;
    const int width = MeasureText(text, size);
    const int x = (GetScreenWidth() - width) / 2;
    DrawRectangleRounded({static_cast<float>(x - 30), static_cast<float>(y - 16),
                          static_cast<float>(width + 60), static_cast<float>(size + 32)},
                         0.3F, 8, panel);
    DrawText(text, x, y, size, colour);
}

} // namespace

void draw_hud(const core::circus::Game& game)
{
    const int width = GetScreenWidth();

    const std::string level = "Level " + std::to_string(game.level_index() + 1) + " / " +
                              std::to_string(game.level_count());
    DrawRectangleRounded(
        {12.0F, 12.0F, static_cast<float>(MeasureText(level.c_str(), 26) + 28), 46.0F}, 0.4F, 8,
        panel);
    DrawText(level.c_str(), 26, 22, 26, text_colour);
    const char* leave = "Esc - menu";
    DrawText(leave, width - MeasureText(leave, 20) - 20, 24, 20, Fade(text_colour, 0.7F));

    const int y = GetScreenHeight() - 64;
    DrawRectangle(0, y - 12, width, 64, panel);
    int x = 24;
    x = draw_words("hold", x, y);
    x = draw_button('A', x, y);
    x = draw_words("walk on two legs", x, y);
    x += 40;
    x = draw_button('X', x, y);
    x = draw_words("then", x, y);
    x = draw_button('B', x, y);
    draw_words("jump with a salto", x, y);

    switch (game.phase())
    {
    case core::circus::GamePhase::Playing:
        if (game.world().elephant().state == core::circus::ElephantState::Standing)
        {
            draw_big_centred("Hold A!", 150, button_colour);
        }
        break;
    case core::circus::GamePhase::LevelFailed:
        draw_big_centred("Oops! Once more...", 150, bad_colour);
        break;
    case core::circus::GamePhase::LevelComplete:
        draw_big_centred("Bravo! Press A", 150, good_colour);
        break;
    case core::circus::GamePhase::CampaignComplete:
        break;
    }
}

} // namespace ce::render::circus
