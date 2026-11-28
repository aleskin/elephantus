#include "render/minigames/circus/circus_play.hpp"

#include "core/minigames/circus/circus.hpp"
#include "core/minigames/circus/level/level.hpp"
#include "render/minigames/circus/hud.hpp"
#include "render/minigames/circus/locations/location.hpp"
#include "render/minigames/circus/scene.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace ce::render::circus
{

namespace
{

// Loads a level and makes sure the game knows how to draw its place.
core::circus::LevelData load_drawable_level(const std::filesystem::path& path)
{
    core::circus::LevelData level = core::circus::load_level(path);
    if (find_location(level.location) == nullptr)
    {
        throw std::runtime_error(path.filename().string() + ": no view for location '" +
                                 level.location + "'");
    }
    return level;
}

sound::Cue cue_for(core::circus::EventKind kind)
{
    switch (kind)
    {
    case core::circus::EventKind::Jumped:
        return sound::Cue::Jump;
    case core::circus::EventKind::Landed:
        return sound::Cue::Land;
    case core::circus::EventKind::HoopPassed:
        return sound::Cue::HoopPassed;
    case core::circus::EventKind::HoopTouched:
    case core::circus::EventKind::Fell:
    case core::circus::EventKind::PodiumHit:
        return sound::Cue::Sad;
    case core::circus::EventKind::LevelFinished:
        return sound::Cue::Applause;
    }
    return sound::Cue::Land;
}

class CircusPlay : public MinigamePlay
{
public:
    CircusPlay(const std::filesystem::path& assets, const core::KvDocument& config,
               core::MinigameSetup setup)
        : game_(core::circus::load_campaign(core::circus::campaign_path(assets)),
                load_drawable_level, config, setup)
    {
    }

    core::Minigame& game() override
    {
        return game_;
    }

    void draw(const core::ButtonLabels& labels) const override
    {
        draw_scene(game_.game().world());
        draw_hud(game_.game(), labels);
    }

    std::vector<sound::Cue> take_cues() override
    {
        std::vector<sound::Cue> cues;
        for (const core::circus::Event& event : game_.take_events())
        {
            cues.push_back(cue_for(event.kind));
        }
        return cues;
    }

private:
    core::circus::CircusMinigame game_;
};

} // namespace

std::unique_ptr<MinigamePlay> start(const std::filesystem::path& assets,
                                    const core::KvDocument& config, core::MinigameSetup setup)
{
    return std::make_unique<CircusPlay>(assets, config, setup);
}

} // namespace ce::render::circus
