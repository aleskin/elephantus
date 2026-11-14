#pragma once

#include "core/text/kv_file.hpp"

namespace ce::core
{

// A circus hoop standing across the path. The ring is perpendicular to the path:
// the elephant goes through it along x.
struct Hoop
{
    // TODO(CEL-00012): hoop size and height are placeholders until Polina decides.
    static constexpr double default_height = 1.6;
    static constexpr double default_radius = 1.0;

    double x = 0.0;
    double height = default_height;
    double radius = default_radius;

    double bottom() const;
    double top() const;
};

// [hoop] section: x (required), height of the ring centre, radius.
Hoop parse_hoop(const KvSection& section);

} // namespace ce::core
