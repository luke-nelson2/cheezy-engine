#pragma once
#include <cstdint>
#include <array>

namespace MoveUtility {
    extern const std::array<uint64_t, 64> KNIGHT_MOVES;
    extern const std::array<uint64_t, 64> KING_MOVES;
    extern const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS;
    extern const std::array<uint8_t, 64> CASTLING_RIGHTS_UPDATE;
}