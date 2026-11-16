#pragma once

#include "core/party/minigame.hpp"

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
};

} // namespace ce::render
