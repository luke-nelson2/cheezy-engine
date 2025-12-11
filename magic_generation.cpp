#include <bitset>
#include <iostream>
#include <cstdint>
#include <cmath>
#include "move_utility.h"

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

void print_bitboard(uint64_t bb) {
    for (int i = 7; i >= 0; i--) {
      for (int j = 0; j<=7; j++) {
        if (bb & (1ULL << (i*8+j))) {
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
        return count_bits((bitboard & (~bitboard+1)) - 1);
    } else {
        return 64;
    }
}

uint64_t set_occupancy(uint16_t index, uint8_t bits_in_mask, uint64_t attack_mask) {
    uint64_t occupancy = 0ULL;

    for (uint8_t count = 0; count < bits_in_mask; count++) {
        uint8_t square = get_ls1b_index(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }

    return occupancy;
}

uint64_t generate_rook_attacks_fly(uint8_t square, uint64_t occupancy) {
    return 0;
}

int main() {
    uint16_t idx = 1021;
    uint64_t perm = set_occupancy(idx, 10, MoveUtility::ROOK_MASK_TABLE[17]);
    std::cout << std::bitset<16>(idx) << std::endl;
    print_bitboard(perm);
    return 0;
}