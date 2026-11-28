#include "render/minigames/circus/hud.hpp"

#include "render/text.hpp"

#include <raylib.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

namespace ce::render::circus
{

namespace
{

constexpr Color panel{0, 0, 0, 150};
constexpr Color text_colour{235, 235, 245, 255};
constexpr Color button_colour{250, 204, 80, 255};
constexpr Color good_colour{120, 230, 120, 255};
constexpr Color bad_colour{255, 110, 100, 255};

// A key or gamepad button as a yellow chip with its name; returns the x right after it.
int draw_button(const std::string& label, int x, int y)
{
    const int size = 24;
    const int height = 36;
    const int width = std::max(height, measure_text(label, size) + 20);
    DrawRectangleRounded({static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                          static_cast<float>(height)},
                         0.5F, 8, button_colour);
    draw_text(label, x + (width - measure_text(label, size)) / 2, y + height / 2 - size / 2 - 1,
              size, BLACK);
    return x + width + 8;
}

const std::string& label_of(const core::ButtonLabels& labels, core::GameButton button)
{
    return labels[static_cast<std::size_t>(button)];
}

int draw_words(const char* words, int x, int y)
{
    const int size = 24;
    draw_text(words, x, y + 4, size, text_colour);
    return x + measure_text(words, size) + 10;
}

void draw_big_centred(std::string_view text, int y, Color colour)
{
    const int size = 64;
    const int width = measure_text(text, size);
    const int x = (GetScreenWidth() - width) / 2;
    DrawRectangleRounded({static_cast<float>(x - 30), static_cast<float>(y - 16),
                          static_cast<float>(width + 60), static_cast<float>(size + 32)},
                         0.3F, 8, panel);
    draw_text(text, x, y, size, colour);
}

// A smaller line under the big one.
void draw_small_centred(std::string_view text, int y)
{
    const int size = 32;
    const int width = measure_text(text, size);
    const int x = (GetScreenWidth() - width) / 2;
    DrawRectangleRounded({static_cast<float>(x - 20), static_cast<float>(y - 10),
                          static_cast<float>(width + 40), static_cast<float>(size + 20)},
                         0.3F, 8, panel);
    draw_text(text, x, y, size, text_colour);
}

// Top left: the level, the points of this attempt, which attempt it is and the record.
void draw_status(const core::circus::Game& game)
{
    const std::string level = "Уровень " + std::to_string(game.level_index() + 1) + " / " +
                              std::to_string(game.level_count());
    const std::string score = "Очки: " + std::to_string(game.score());
    const std::string attempt =
        "Попытка " + std::to_string(game.attempt()) + " / " + std::to_string(game.attempt_count());
    const std::string record = "Рекорд: " + std::to_string(game.record());
    const int size = 28;
    const int gap = 36;
    int x = 26;
    const int width = measure_text(level, size) + measure_text(score, size) +
                      measure_text(attempt, size) + measure_text(record, size) + 3 * gap;
    DrawRectangleRounded({12.0F, 12.0F, static_cast<float>(width + 28), 46.0F}, 0.4F, 8, panel);
    draw_text(level, x, 20, size, text_colour);
    x += measure_text(level, size) + gap;
    draw_text(score, x, 20, size, button_colour);
    x += measure_text(score, size) + gap;
    draw_text(attempt, x, 20, size, text_colour);
    x += measure_text(attempt, size) + gap;
    draw_text(record, x, 20, size, good_colour);
}

} // namespace

void draw_hud(const core::circus::Game& game, const core::ButtonLabels& labels)
{
    const int width = GetScreenWidth();

    draw_status(game);
    const char* leave = "Esc — меню";
    draw_text(leave, width - measure_text(leave, 22) - 20, 22, 22, Fade(text_colour, 0.7F));

    const int y = GetScreenHeight() - 64;
    DrawRectangle(0, y - 12, width, 64, panel);
    int x = 24;
    x = draw_words("держи", x, y);
    x = draw_button(label_of(labels, core::GameButton::A), x, y);
    x = draw_words("— идти на двух ногах", x, y);
    x += 40;
    x = draw_button(label_of(labels, core::GameButton::X), x, y);
    x = draw_words("потом", x, y);
    x = draw_button(label_of(labels, core::GameButton::B), x, y);
    draw_words("— прыжок с сальтухой", x, y);

    switch (game.phase())
    {
    case core::circus::GamePhase::Playing:
        if (game.world().elephant().state == core::circus::ElephantState::Standing)
        {
            const std::string hold = "Держи " + label_of(labels, core::GameButton::A) + "!";
            draw_big_centred(hold, 150, button_colour);
        }
        break;
    case core::circus::GamePhase::LevelFailed:
        draw_big_centred(game.attempts_left() > 0 ? "Ой! Ещё разок…" : "Ой!", 150, bad_colour);
        draw_small_centred("Очки за попытку: " + std::to_string(game.score()), 250);
        break;
    case core::circus::GamePhase::LevelComplete:
    {
        const std::string next = "Браво! Нажми " + label_of(labels, core::GameButton::A);
        draw_big_centred(next, 150, good_colour);
        break;
    }
    case core::circus::GamePhase::CampaignComplete:
        draw_big_centred("Браво! Очки: " + std::to_string(game.score()), 150, good_colour);
        draw_small_centred("Ещё попытка — нажми " + label_of(labels, core::GameButton::A), 250);
        break;
    case core::circus::GamePhase::GameOver:
    case core::circus::GamePhase::Done:
        draw_big_centred("Попытки кончились! Лучшие очки: " + std::to_string(game.best()), 150,
                         button_colour);
        draw_small_centred("Рекорд: " + std::to_string(game.record()) + ". Нажми " +
                               label_of(labels, core::GameButton::A),
                           250);
        break;
    }
}

} // namespace ce::render::circus
