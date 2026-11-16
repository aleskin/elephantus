#pragma once

#include "core/minigames/circus/settings/tunables.hpp"
#include "core/text/kv_file.hpp"

namespace ce::core::circus
{

// The circus numbers in the game config: [circus.elephant], [circus.buttons], [circus.game].
// A missing key keeps the value from the code; an unknown [circus.…] section or key is an
// error, and every number must be above zero. Other sections are not looked at.
Tunables read_tunables(const KvDocument& config);

// MinigameInfo::check_config for the circus.
void check_config(const KvDocument& config);

} // namespace ce::core::circus
