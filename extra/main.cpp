#include "move_utility.h"
#include "position.h"
#include <iostream>
#include <cstdint>

using namespace MoveUtility;

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
  // std::string fen_string = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
  // Position test_pos(fen_string);
  // test_pos.print_position();
  // std::cout << (int)test_pos.castling_rights << std::endl;

  uint64_t att = MoveUtility::PAWN_ATTACKS[0][50];
  print_bitboard(att);

  return 0;
}