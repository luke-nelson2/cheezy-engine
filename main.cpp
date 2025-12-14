#include "move_utility.h"
// #include "position.h"
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
  print_bitboard(FILE_E);
  return 0;
}