#pragma once

#include "core/party/flow.hpp"

namespace ce::render
{

// The screens around the mini-games, one file each. They draw the whole frame.
// raylib's built-in font has no Cyrillic, so the text is English until CEL-00034.

// TODO(CEL-00035): what Polina wants to see on the title; for now the name and a grey
// elephant. `time` is seconds since the title appeared, for the blinking "press A".
void draw_title(double time);
void draw_main_menu(const core::PartyFlow& flow);
void draw_minigame_list(const core::PartyFlow& flow);

} // namespace ce::render
