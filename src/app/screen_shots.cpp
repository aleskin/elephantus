#include "app/screen_shots.hpp"

#include "core/party/flow.hpp"
#include "render/screens/screens.hpp"

#include <raylib.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace ce::app
{

namespace
{

void save(RenderTexture2D target, const std::function<void()>& draw,
          const std::filesystem::path& path)
{
    BeginTextureMode(target);
    draw();
    EndTextureMode();

    Image image = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);
    const bool saved = ExportImage(image, path.string().c_str());
    UnloadImage(image);
    if (!saved)
    {
        throw std::runtime_error("cannot save " + path.string());
    }
}

} // namespace

void save_screens(std::vector<core::MinigameInfo> minigames, const std::filesystem::path& dir)
{
    std::filesystem::create_directories(dir);
    core::PartyFlow flow{std::move(minigames)};
    core::MenuInput confirm;
    confirm.confirm = true;

    const RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    try
    {
        save(
            target,
            []
            {
                render::draw_title(0.0);
            },
            dir / "title.png");
        flow.input(confirm);
        save(
            target,
            [&]
            {
                render::draw_main_menu(flow);
            },
            dir / "main_menu.png");
        flow.input(confirm);
        save(
            target,
            [&]
            {
                render::draw_minigame_list(flow);
            },
            dir / "minigame_list.png");
    }
    catch (...)
    {
        UnloadRenderTexture(target);
        throw;
    }
    UnloadRenderTexture(target);
}

} // namespace ce::app
