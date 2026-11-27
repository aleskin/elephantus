#include "sound/sounds.hpp"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <numbers>
#include <vector>

namespace ce::sound
{

namespace
{

constexpr int sample_rate = 44100;
constexpr std::size_t cue_count = 8;
static_assert(static_cast<std::size_t>(Cue::LevelDone) + 1 == cue_count);

enum class Shape
{
    Sine,
    Square,
    Triangle,
    Noise,
};

// One piece of a sound: a tone gliding from one pitch to another.
struct Note
{
    double from_hz;
    double to_hz;
    double seconds;
    Shape shape;
    double volume;
};

double wave(Shape shape, double phase, std::uint32_t& noise)
{
    switch (shape)
    {
    case Shape::Sine:
        return std::sin(2.0 * std::numbers::pi * phase);
    case Shape::Square:
        return phase < 0.5 ? 0.6 : -0.6;
    case Shape::Triangle:
        return 4.0 * std::abs(phase - 0.5) - 1.0;
    case Shape::Noise:
        noise = noise * 1664525U + 1013904223U;
        return static_cast<double>(noise >> 8U) / static_cast<double>(1U << 23U) - 1.0;
    }
    return 0.0;
}

std::vector<short> synthesize(std::initializer_list<Note> notes)
{
    std::vector<short> samples;
    std::uint32_t noise = 12345U;
    for (const Note& note : notes)
    {
        const int count = static_cast<int>(note.seconds * sample_rate);
        double phase = 0.0;
        for (int i = 0; i < count; ++i)
        {
            const double t = static_cast<double>(i) / count;
            const double hz = note.from_hz + (note.to_hz - note.from_hz) * t;
            phase += hz / sample_rate;
            phase -= std::floor(phase);
            // A quick start and a soft fade, so the sound does not click.
            const double attack = std::min(1.0, i / (0.004 * sample_rate));
            const double envelope = attack * (1.0 - t) * (1.0 - t);
            const double value = wave(note.shape, phase, noise) * envelope * note.volume;
            samples.push_back(static_cast<short>(value * 32767.0 * 0.8));
        }
    }
    return samples;
}

std::vector<short> make(Cue cue)
{
    switch (cue)
    {
    case Cue::MenuMove:
        return synthesize({{880, 880, 0.05, Shape::Square, 0.25}});
    case Cue::MenuSelect:
        return synthesize(
            {{660, 660, 0.06, Shape::Square, 0.3}, {990, 990, 0.12, Shape::Square, 0.3}});
    case Cue::MenuBack:
        return synthesize(
            {{550, 550, 0.06, Shape::Square, 0.3}, {370, 370, 0.12, Shape::Square, 0.3}});
    case Cue::Jump:
        return synthesize({{280, 900, 0.22, Shape::Triangle, 0.7}});
    case Cue::Land:
        return synthesize({{140, 50, 0.12, Shape::Sine, 0.9}, {0, 0, 0.05, Shape::Noise, 0.15}});
    case Cue::HoopPassed:
        return synthesize({{1047, 1047, 0.09, Shape::Sine, 0.6},
                           {1319, 1319, 0.09, Shape::Sine, 0.6},
                           {1568, 1568, 0.3, Shape::Sine, 0.6}});
    case Cue::Oops:
        return synthesize(
            {{520, 380, 0.25, Shape::Triangle, 0.7}, {400, 160, 0.5, Shape::Triangle, 0.7}});
    case Cue::LevelDone:
        return synthesize({{523, 523, 0.12, Shape::Square, 0.35},
                           {659, 659, 0.12, Shape::Square, 0.35},
                           {784, 784, 0.12, Shape::Square, 0.35},
                           {1047, 1047, 0.5, Shape::Square, 0.35}});
    }
    return {};
}

} // namespace

struct Sounds::Bank
{
    std::array<Sound, cue_count> sounds{};
};

Sounds::Sounds()
{
    InitAudioDevice();
    if (!IsAudioDeviceReady())
    {
        return;
    }
    bank_ = std::make_unique<Bank>();
    for (std::size_t i = 0; i < cue_count; ++i)
    {
        std::vector<short> samples = make(static_cast<Cue>(i));
        Wave wave{};
        wave.frameCount = static_cast<unsigned int>(samples.size());
        wave.sampleRate = sample_rate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples.data();
        bank_->sounds[i] = LoadSoundFromWave(wave);
    }
}

Sounds::~Sounds()
{
    if (bank_)
    {
        for (const Sound& sound : bank_->sounds)
        {
            UnloadSound(sound);
        }
        bank_.reset();
    }
    if (IsAudioDeviceReady())
    {
        CloseAudioDevice();
    }
}

void Sounds::play(Cue cue)
{
    if (bank_)
    {
        PlaySound(bank_->sounds[static_cast<std::size_t>(cue)]);
    }
}

} // namespace ce::sound
