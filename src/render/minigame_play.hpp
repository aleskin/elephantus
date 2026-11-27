#pragma once

#include "core/party/minigame.hpp"
#include "sound/cue.hpp"

#include <vector>

namespace ce::render
{

// A running mini-game together with its look. Every mini-game has one in
// render/minigames/<id>/.
class MinigamePlay
{
public:
    MinigamePlay() = default;
    MinigamePlay(const MinigamePlay&) = delete;
    MinigamePlay& operator=(const MinigamePlay&) = delete;
    virtual ~MinigamePlay() = default;

    virtual core::Minigame& game() = 0;
    // The whole frame, between BeginDrawing/EndDrawing or into a render texture.
    virtual void draw() const = 0;
    // The sounds for what happened since the last call.
    virtual std::vector<sound::Cue> take_cues() = 0;
};

} // namespace ce::render
