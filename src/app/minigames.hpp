#pragma once

#include "core/party/minigame.hpp"
#include "core/text/kv_file.hpp"
#include "render/minigame_play.hpp"

#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace ce::app
{

// A mini-game of Elephantus: its rules (core/minigames/<id>/) and its look
// (render/minigames/<id>/). A new mini-game = two folders + one line in minigames.cpp.
struct MinigameModule
{
    const core::MinigameInfo* info;
    std::unique_ptr<render::MinigamePlay> (*start)(const std::filesystem::path& assets,
                                                   const core::KvDocument& config,
                                                   core::MinigameSetup setup);
};

// In menu order.
std::span<const MinigameModule> minigames();
// nullptr if there is no mini-game with this id.
const MinigameModule* find_minigame(std::string_view id);
// For the game config: which [<id>.…] sections exist and how to check them.
std::vector<core::MinigameInfo> minigame_infos();

} // namespace ce::app
