#include "magic_rng.h"
#include "move_utility.h"
#include <cstdint>
#include <iostream>

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square)                                              \
  (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

enum uint8t {
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

void print_bitboard(uint64_t bb) {
  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j <= 7; j++) {
      if (bb & (1ULL << (i * 8 + j))) {
        std::cout << "1 "; // Print '1' if bit is set
      } else {
        std::cout << ". "; // Print '.' if bit is clear
      }
    }
    std::cout << std::endl;
  }
  std::cout << "\n";
}

uint8_t count_bits(uint64_t bitboard) {
  uint8_t count = 0;

  while (bitboard) {
    count++;
    bitboard &= bitboard - 1;
  }

  return count;
}

uint8_t get_ls1b_index(uint64_t bitboard) {
  if (bitboard != 0) {
    return count_bits((bitboard & (~bitboard + 1)) - 1);
  } else {
    return 64;
  }
}

uint64_t set_occupancy(uint16_t index, uint8_t bits_in_mask,
                       uint64_t attack_mask) {
  uint64_t occupancy = 0ULL;

  for (uint8_t count = 0; count < bits_in_mask; count++) {
    uint8_t square = get_ls1b_index(attack_mask);
    pop_bit(attack_mask, square);
    if (index & (1 << count))
      occupancy |= (1ULL << square);
  }

  return occupancy;
}

uint64_t generate_rook_attacks_rays(uint8_t square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  uint8_t target_rank = square / 8;
  uint8_t target_file = square % 8;
  int rank, file;

  // Right
  for (rank = target_rank + 1; rank < 8; rank++) {
    attacks |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy)
      break;
  }
  // Left
  for (rank = target_rank - 1; rank >= 0; rank--) {
    attacks |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy)
      break;
  }
  // Up
  for (file = target_file + 1; file < 8; file++) {
    attacks |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy)
      break;
  }
  // Down
  for (file = target_file - 1; file >= 0; file--) {
    attacks |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy)
      break;
  }

  return attacks;
}

uint64_t generate_bishop_attacks_rays(uint8_t square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  uint8_t target_rank = square / 8;
  uint8_t target_file = square % 8;
  int rank, file;

  // NorthEast
  for (rank = target_rank + 1, file = target_file + 1; rank < 8 && file < 8;
       rank++, file++) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // NorthWest
  for (rank = target_rank + 1, file = target_file - 1; rank < 8 && file > -1;
       rank++, file--) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // SouthWest
  for (rank = target_rank - 1, file = target_file - 1; rank > -1 && file > -1;
       rank--, file--) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // SouthEast
  for (rank = target_rank - 1, file = target_file + 1; rank > -1 && file < 8;
       rank--, file++) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }

  return attacks;
}

uint64_t find_magic(uint8_t square, uint8_t relevant_bits, uint8_t bishop) {

  std::array<uint64_t, 4096> attacks;
  std::array<uint64_t, 4096> occupancies;
  std::array<uint64_t, 4096> used_attacks;

  uint16_t occupancy_variations = 1 << relevant_bits;
  uint64_t mask_attack = bishop ? MoveUtility::BISHOP_MASK_TABLE[square]
                                : MoveUtility::ROOK_MASK_TABLE[square];

  // Generate occupancy variations and their respective valid attacks
  for (uint16_t count = 0; count < occupancy_variations; count++) {
    occupancies[count] = set_occupancy(count, relevant_bits, mask_attack);
    attacks[count] =
        bishop ? generate_bishop_attacks_rays(square, occupancies[count])
               : generate_rook_attacks_rays(square, occupancies[count]);
  }

  for (uint64_t random_count = 0; random_count < 100'000'000; random_count++) {
    uint64_t magic_candidate = MagicRNG::get().random_sparse();

    if (count_bits((mask_attack * magic_candidate) & 0xFF00000000000000ULL) < 6)
      continue;

    used_attacks.fill(0);

    uint16_t count, fail;

    for (count = 0, fail = 0; !fail && count < occupancy_variations; count++) {
      uint64_t magic_index =
          ((occupancies[count] * magic_candidate) >> (64 - relevant_bits));

      if (used_attacks[magic_index] == 0ULL) {
        used_attacks[magic_index] = attacks[count];
      } else if (used_attacks[magic_index] != attacks[count]) {
        fail = 1;
      }
    }

    if (!fail)
      return magic_candidate;
  }

  std::cout << "Failed" << std::endl;
  return 0;
}

std::array<uint64_t, 64> find_all_magics(uint8_t bishop) {
  std::array<uint64_t, 64> all_magics;
  std::array<uint8_t, 64> relevant_bits =
      bishop ? MoveUtility::BISHOP_RELEVANT_BITS : MoveUtility::ROOK_RELEVANT_BITS;

  for (uint8_t i = 0; i < 64; i++) {
    all_magics[i] = find_magic(i, relevant_bits[i], bishop);
    printf("0x%llX", (unsigned long long)all_magics[i]);
    std::cout << ',' << std::endl;
  }

  return all_magics;
}

int main() {
  find_all_magics(1);
  return 0;
}