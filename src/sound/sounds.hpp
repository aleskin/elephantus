#pragma once

#include "sound/cue.hpp"

#include <memory>

namespace ce::sound
{

// Opens the audio device and makes every sound in code, without sound files. Without an
// audio device the game stays silent and plays on.
class Sounds
{
public:
    Sounds();
    Sounds(const Sounds&) = delete;
    Sounds& operator=(const Sounds&) = delete;
    ~Sounds();

    void play(Cue cue);

private:
    struct Bank;
    std::unique_ptr<Bank> bank_;
};

} // namespace ce::sound
