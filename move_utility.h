#pragma once
#include <array>
#include <cstdint>

namespace MoveUtility {
extern const std::array<uint64_t, 64> ROOK_MAGICS;
extern const std::array<uint64_t, 64> BISHOP_MAGICS;
extern const std::array<uint8_t, 64> ROOK_RELEVANT_BITS;
extern const std::array<uint8_t, 64> BISHOP_RELEVANT_BITS;
extern const std::array<uint64_t, 64> KNIGHT_MOVES;
extern const std::array<uint64_t, 64> KING_MOVES;
extern const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS;
extern const std::array<uint8_t, 64> CASTLING_RIGHTS_UPDATE;
extern const std::array<uint64_t, 64> ROOK_MASK_TABLE;
extern const std::array<uint64_t, 64> BISHOP_MASK_TABLE;
} // namespace MoveUtility