#include "render/text.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace ce::render
{

namespace
{

// Glyphs are made once at this size and scaled down smoothly.
constexpr int glyph_size = 96;

const Font* current = nullptr;

std::vector<int> game_codepoints()
{
    std::vector<int> codepoints;
    for (int c = 32; c < 127; ++c)
    {
        codepoints.push_back(c);
    }
    for (int c = 0x410; c <= 0x44F; ++c)
    {
        codepoints.push_back(c);
    }
    for (const int c : {0x401, 0x451, 0xAB, 0xBB, 0x2014, 0x2026, 0x2116})
    {
        codepoints.push_back(c);
    }
    return codepoints;
}

Font font()
{
    return current != nullptr ? *current : GetFontDefault();
}

float spacing(int size)
{
    return current != nullptr ? 0.0F : static_cast<float>(size) / 10.0F;
}

} // namespace

GameFont::GameFont(const std::filesystem::path& assets)
{
    const std::filesystem::path path = assets / "fonts" / "Nunito-ExtraBold.ttf";
    std::vector<int> codepoints = game_codepoints();
    font_ = LoadFontEx(path.string().c_str(), glyph_size, codepoints.data(),
                       static_cast<int>(codepoints.size()));
    if (!IsFontValid(font_) || font_.texture.id == GetFontDefault().texture.id)
    {
        throw std::runtime_error("cannot load the font " + path.string());
    }
    GenTextureMipmaps(&font_.texture);
    SetTextureFilter(font_.texture, TEXTURE_FILTER_TRILINEAR);
    current = &font_;
}

GameFont::~GameFont()
{
    current = nullptr;
    UnloadFont(font_);
}

void draw_text(std::string_view text, int x, int y, int size, Color colour)
{
    const std::string copy{text};
    DrawTextEx(font(), copy.c_str(), {static_cast<float>(x), static_cast<float>(y)},
               static_cast<float>(size), spacing(size), colour);
}

int measure_text(std::string_view text, int size)
{
    const std::string copy{text};
    return static_cast<int>(
        MeasureTextEx(font(), copy.c_str(), static_cast<float>(size), spacing(size)).x);
}

void draw_text_centred(std::string_view text, int y, int size, Color colour)
{
    draw_text(text, (GetScreenWidth() - measure_text(text, size)) / 2, y, size, colour);
}

} // namespace ce::render
