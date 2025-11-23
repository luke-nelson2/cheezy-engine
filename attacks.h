#pragma once
#include <cstdint>
#include <array>

namespace Attacks {
    extern const std::array<uint64_t, 64> KNIGHT_MOVES;
    extern const std::array<uint64_t, 64> KING_MOVES;
    extern const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS;
}