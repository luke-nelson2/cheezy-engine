#include "attacks.h"
#include <cstdint>
// #include <bitset>
#include <iostream>
#include <sys/types.h>

struct Move {
  uint16_t move_data;

  Move(uint8_t from_sq, uint8_t to_sq, uint8_t flags=0) {
    move_data = (uint16_t)from_sq | ((uint16_t)to_sq << 6) | ((uint16_t)flags << 12);
  }
  
  inline uint8_t get_from_sq() const {
    return move_data & 0x3F;
  }

  inline uint8_t get_to_sq() const {
    return (move_data >> 6) & 0x3F;
  }

  inline uint8_t get_flags() const {
    return (move_data >> 12) & 0xF;
  }

  // FLAGS
  //   0 - NORMAL MOVE
  //   1 - PROMOTION
  //   2 - CASTLING
  //   3 - EN PASSANT
  //   PIECES TO PROMOTE TO: KNIGHT, BISHOP, ROOK, QUEEN
};

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

  std::array<uint8_t, 64> piece_list;


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

    // PAWNS = 0/0b0    0
    // KNIGHT = 1/0b1   2
    // BISHOP = 2/0b10  4
    // ROOK = 3/0b11    6
    // QUEEN = 4/0b100  8
    // KING = 5/0b101   10
    // ADD ONE IF BLACK

    for (int i = 0; i<64; i++) {
      piece_list[i] = 0;
    }

    // PAWNS 
    for (int i = 8; i<16; i++) {
      // WHITE 0b00
      piece_list[i] = 0;
      // BLACK 0b00
      piece_list[i+48] = 1;
    }

    // WHITE KNIGHTS 0b10
    piece_list[1] = 2;
    piece_list[6] = 2;

    // BLACK KNIGHTS 0b11
    piece_list[57] = 3;
    piece_list[62] = 3;

    // WHITE BISHOPS
    piece_list[2] = 4;
    piece_list[5] = 4;

    // BLACK BISHOPS
    piece_list[58] = 5;
    piece_list[61] = 5;

    // WHITE ROOKS
    piece_list[0] = 6;
    piece_list[7] = 6;

    // BLACK ROOKS
    piece_list[56] = 7;
    piece_list[63] = 7;

    // WHITE QUEEN
    piece_list[3] = 8;

    // BLACK QUEEN
    piece_list[59] = 9;

    // WHITE KING
    piece_list[4] = 10;

    // BLACK KING
    piece_list[60] = 11;
  }
  
  void make_move(Move move){
    uint8_t from_sq = move.get_from_sq();
    uint8_t to_sq = move.get_to_sq();
    uint8_t origin_piece_type = piece_list[from_sq];
  }

};



struct UndoInfo {
  Move move;
  uint8_t captured_piece_type;
  uint8_t castling_rights;
  uint8_t en_passant_sq;
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
//   unmake moves while going back up the tree
// Convert moves to UCI (e2e4)
//   perform XOR operation on 2 aggregate bitboards

// Very much later TODO:
// Artificially deflate the elo of the engine
// Setup to be fully compliant with UCI and able to play other engines



int main(){
  //Position board{};
  //print_bitboard(board.black_pawn);
  // print_bitboard(Attacks::KING_MOVES[45]);
  Move move1(54,32);
  std::cout << move1.move_data << std::endl;
  std::cout << (int)(move1.get_to_sq()) << std::endl;
  return 0;
}
