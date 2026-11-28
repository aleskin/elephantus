#pragma once

#include "core/text/kv_file.hpp"

namespace ce::core::circus
{

// The finish: a low box the elephant jumps onto. Landing on its top finishes the level.
struct Podium
{
    // TODO(CEL-00012): podium size is a placeholder until Polina decides.
    static constexpr double default_width = 2.0;
    static constexpr double default_height = 0.5;

    double x = 0.0; // middle of the podium along the path
    double width = default_width;
    double height = default_height;

    double left() const;
    double right() const;
};

// [podium] section: x (required), width, height.
Podium parse_podium(const KvSection& section);

} // namespace ce::core::circus
