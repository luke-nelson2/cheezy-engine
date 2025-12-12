#pragma once
#include <array>
#include <cstdint>

namespace MoveUtility {

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square)                                              \
  (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

enum Square : uint8_t {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
  NO_SQUARE
};

struct MagicEntry {
  uint64_t mask;
  uint64_t magic;
  uint64_t offset;
  uint64_t shift;
};

extern const std::array<MagicEntry, 64> ROOK_MAGIC_ENTRY;
extern const std::array<MagicEntry, 64> BISHOP_MAGIC_ENTRY;
extern const std::array<uint64_t, 102400> ROOK_ATTACKS;
extern const std::array<uint64_t, 5248> BISHOP_ATTACKS;
extern const std::array<uint64_t, 64> KNIGHT_MOVES;
extern const std::array<uint64_t, 64> KING_MOVES;
extern const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS;
extern const std::array<uint8_t, 64> CASTLING_RIGHTS_UPDATE;

inline uint64_t get_rook_attacks(uint8_t square, uint64_t occupancy) {
  const MagicEntry& m = ROOK_MAGIC_ENTRY[square];
  uint64_t masked_occ = m.mask & occupancy;
  uint16_t magic_idx = (masked_occ * m.magic) >> m.shift;

  return ROOK_ATTACKS[magic_idx + m.offset];
}

inline uint64_t get_bishop_attacks(uint8_t square, uint64_t occupancy) {
  const MagicEntry& m = BISHOP_MAGIC_ENTRY[square];
  uint64_t masked_occ = m.mask & occupancy;
  uint16_t magic_idx = (masked_occ * m.magic) >> m.shift;

  return BISHOP_ATTACKS[magic_idx + m.offset];
}
} // namespace MoveUtility