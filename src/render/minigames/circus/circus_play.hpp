#pragma once

#include "core/party/minigame.hpp"
#include "core/text/kv_file.hpp"
#include "render/minigame_play.hpp"

#include <filesystem>
#include <memory>

namespace ce::render::circus
{

// Loads the circus levels from the assets and makes sure every place has a look.
std::unique_ptr<MinigamePlay> start(const std::filesystem::path& assets,
                                    const core::KvDocument& config, core::MinigameSetup setup);

} // namespace ce::render::circus
