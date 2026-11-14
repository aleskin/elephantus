#pragma once

#include "core/input/input_frame.hpp"
#include "core/text/kv_file.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace ce::core
{

// A scripted run: button presses and screenshots at given moments, then the end.
// Used by `make shots` so the game can be checked without a person at the keyboard.
//
//   [press]            [release]          [shot]             [end]
//   at = 0.5           at = 0.6           at = 1.0           at = 2.0
//   button = jump      button = jump      name = landed
//
// Times are seconds from the start; they are turned into logic steps.
struct ScriptAction
{
    enum class Kind
    {
        Press,
        Release,
        Shot,
    };

    Kind kind = Kind::Shot;
    long step = 0;
    GameButton button = GameButton::Jump;
    std::string name;
};

struct Script
{
    std::vector<ScriptAction> actions; // sorted by step
    long end_step = 0;                 // the run stops after this step
};

Script parse_script(const KvDocument& document, double step_rate);
Script load_script(const std::filesystem::path& path, double step_rate);

// Plays a script step by step: which buttons are down and which shots to take.
class ScriptPlayer
{
public:
    explicit ScriptPlayer(Script script);

    bool done() const;
    long step() const;
    // Applies the presses and releases of the current step.
    const ButtonsDown& buttons();
    // Shot names for the current step (after the logic step ran).
    std::vector<std::string> shots() const;
    void next();

private:
    Script script_;
    long step_ = 0;
    std::size_t next_action_ = 0;
    ButtonsDown down_{};
};

} // namespace ce::core
