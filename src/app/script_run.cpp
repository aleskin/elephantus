#include "app/script_run.hpp"

#include "core/input/controls.hpp"
#include "core/input/input_frame.hpp"
#include "core/script/script.hpp"
#include "core/time/fixed_step.hpp"

#include <raylib.h>

#include <stdexcept>
#include <string>

namespace ce::app
{

namespace
{

void save_frame(const render::MinigamePlay& play, RenderTexture2D target,
                const std::filesystem::path& path)
{
    BeginTextureMode(target);
    play.draw(core::control_labels(core::default_controls(core::PadLayout::Nintendo), false,
                                   core::PadLayout::Nintendo));
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

void run_script(render::MinigamePlay& play, const std::filesystem::path& script_path,
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
            play.game().step({&frame, 1}, clock.dt());
            for (const std::string& name : player.shots())
            {
                save_frame(play, target, shots_dir / (prefix + name + ".png"));
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
