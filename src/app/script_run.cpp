#include "app/script_run.hpp"

#include "core/input/input_frame.hpp"
#include "core/script/script.hpp"
#include "core/world/fixed_step.hpp"
#include "render/scene.hpp"

#include <raylib.h>

#include <stdexcept>
#include <string>

namespace ce::app
{

namespace
{

void save_frame(const core::World& world, RenderTexture2D target, const std::filesystem::path& path)
{
    BeginTextureMode(target);
    render::draw_scene(world);
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

void run_script(core::Game& game, const std::filesystem::path& script_path,
                const std::filesystem::path& shots_dir)
{
    const core::FixedStep clock;
    core::ScriptPlayer player{core::load_script(script_path, 1.0 / clock.dt())};
    std::filesystem::create_directories(shots_dir);
    const std::string prefix = script_path.stem().string() + "_";

    const RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    core::InputTracker tracker;
    try
    {
        for (; !player.done(); player.next())
        {
            const core::InputFrame frame = tracker.next(player.buttons());
            game.step(frame, clock.dt());
            for (const std::string& name : player.shots())
            {
                save_frame(game.world(), target, shots_dir / (prefix + name + ".png"));
            }
        }
    }
    catch (...)
    {
        UnloadRenderTexture(target);
        throw;
    }
    UnloadRenderTexture(target);
}

} // namespace ce::app
