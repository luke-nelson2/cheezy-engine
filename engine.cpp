#include <cstdint>
#include <bitset>
#include <iostream>

class Position{
public:
  uint64_t white_pawn;
  uint64_t white_knight;
  uint64_t white_bishop;
  uint64_t white_rook;
  uint64_t white_queen;
  uint64_t white_king;

  uint64_t black_pawn;
  uint64_t black_knight;
  uint64_t black_bishop;
  uint64_t black_rook;
  uint64_t black_queen;
  uint64_t black_king;

  unsigned char c_rights;
  uint64_t ep_target;
public:
  Position() {
    white_pawn = 0xFF00ULL;
    white_knight = 0x42ULL;
    white_bishop = 0x24ULL;
    white_rook = 0x81ULL;
    white_queen = 0x10ULL;
    white_king = 0x8ULL;

    black_pawn = 0xFF'00'00'00'00'00'00ULL;
    black_knight = 0x42'00'00'00'00'00'00'00ULL;
    black_bishop = 0x24'00'00'00'00'00'00'00ULL;
    black_rook = 0x81'00'00'00'00'00'00'00ULL;
    black_queen = 0x10'00'00'00'00'00'00'00ULL;
    black_king = 0x8'00'00'00'00'00'00'00ULL;
  }
  void print_bitboard(uint64_t bb) {
    for (int i = 63; i >= 0; i--) {
        if (bb & (1ULL >> i)) {
            std::cout << "1 "; // Print '1' if bit is set
        } else {
            std::cout << ". "; // Print '.' if bit is clear
        }

        // Print a newline after every 8 bits (after h-file of a rank)
        if (i % 8 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
  }
};

class Move{
public:
  uint16_t move_value;
private:
  uint64_t Knight_Attacks[64];

};

// TODO:
// Init bitboards
// Find all magic numbers
// Generate all possible moves
//   Magic bitboards
// How to store moves?
//   Maybe just perform the attack and just store the board state?
// Get evaluation for the state of the board
//   Use positional evaluation
// minimax search with alpha beta pruning
// Convert moves to UCI (e2e4)
//   perform XOR operation on 2 aggregate bitboards

// Very much later TODO:
// Artificially deflate the elo of the engine
// Setup to be fully compliant with UCI and able to play other engines

int main(){
  Position board{};
  board.print_bitboard(board.white_king);
  return 0;
}
