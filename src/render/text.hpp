#pragma once

#include <raylib.h>

#include <filesystem>
#include <string_view>

namespace ce::render
{

// The game font with Latin and Russian letters, assets/fonts/Nunito-ExtraBold.ttf. While it
// is alive, draw_text and measure_text use it; otherwise raylib's built-in font (no Russian).
class GameFont
{
public:
    // Throws std::runtime_error when the font cannot be loaded.
    explicit GameFont(const std::filesystem::path& assets);
    GameFont(const GameFont&) = delete;
    GameFont& operator=(const GameFont&) = delete;
    ~GameFont();

private:
    Font font_{};
};

// UTF-8 text; (x, y) is its top left corner, size is the line height in pixels.
void draw_text(std::string_view text, int x, int y, int size, Color colour);
int measure_text(std::string_view text, int size);
// Centred across the screen.
void draw_text_centred(std::string_view text, int y, int size, Color colour);

} // namespace ce::render
