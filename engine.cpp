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

  Position() {
    white_pawn = 0xFF00ULL;
    white_knight = 0x42ULL;
    white_bishop = 0x24ULL;
    white_rook = 0x81ULL;
    white_king = 0x10ULL;
    white_queen = 0x8ULL;

    black_pawn = 0xFF'00'00'00'00'00'00ULL;
    black_knight = 0x42'00'00'00'00'00'00'00ULL;
    black_bishop = 0x24'00'00'00'00'00'00'00ULL;
    black_rook = 0x81'00'00'00'00'00'00'00ULL;
    black_king = 0x10'00'00'00'00'00'00'00ULL;
    black_queen = 0x8'00'00'00'00'00'00'00ULL;
  }
  
};

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

class Attack{
public:
  uint16_t move_value;
  static uint64_t knight_attacks[64];

  static void init_knight_table(){
    int idx;
    
    for (int rank=0; rank<8; rank++){
      for (int file=0; file<8; file++){
        uint64_t mask{0x0};
        idx = file*8+rank;
        uint64_t piece_bit = 1ULL << idx;
        uint64_t target;

        // SSW
        if (rank>0 && file>1) {
          target = piece_bit >> 17;
          mask |= target;
        }

        // SSE
        if (rank<7 && file>1) {
          target = piece_bit >> 15;
          mask |= target;
        }

        // ESE
        if (rank<6 && file>0) {
          target = piece_bit >> 6;
          mask |= target;
        }

        // ENE
        if (rank<6 && file<7) {
          target = piece_bit << 10;
          mask |= target;
        }

        // NNE
        if (rank<7 && file<6) {
          target = piece_bit << 17;
          mask |= target;
        }

        // NNW
        if (rank>0 && file<6) {
          target = piece_bit << 15;
          mask |= target;
        }

        // WNW
        if (rank>1 && file<7) {
          target = piece_bit << 6;
          mask |= target;
        }

        // WSW
        if (rank>1 && file>0) {
          target = piece_bit >> 10;
          mask |= target;
        }


        knight_attacks[idx] = mask;
      }
    }
  }

};

uint64_t Attack::knight_attacks[64];

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
  //Position board{};
  //print_bitboard(board.black_pawn);
  Attack::init_knight_table();
  print_bitboard(Attack::knight_attacks[63]);
  return 0;
}
