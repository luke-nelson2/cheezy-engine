#include "move_utility.h"
#include "position.h"
#include <iostream>

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

int main() {
  // Position test{};
  // Move move(8,16);
  // test.make_move(move);
  // print_bitboard(test.total_bb);
  // std::cout << (int)test.en_passant_sq << '\n';
  print_bitboard(MoveUtility::ROOK_MASK_TABLE[2]);
}