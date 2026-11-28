#pragma once

namespace ce::sound
{

// The sounds of the game. Menus and mini-games say which one to play; how it sounds is
// decided in sound/sounds.cpp.
// TODO(CEL-00044): Polina chooses or sings the real sounds.
enum class Cue
{
    MenuMove,
    MenuSelect,
    MenuBack,
    Jump,
    Land,
    HoopPassed,
    Sad,      // a fall: a sad melody
    Applause, // on the podium: a fanfare and the circus claps
};

} // namespace ce::sound
