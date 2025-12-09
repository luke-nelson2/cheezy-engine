#include "move_utility.h"
#include <cstdint>
// #include <bitset>
#include <iostream>
#include <sys/types.h>

enum Piece : uint8_t {
  WHITE = 0,
  BLACK = 1,

  WHITE_PAWN = 0,
  BLACK_PAWN = 1,
  WHITE_KNIGHT = 2,
  BLACK_KNIGHT = 3,
  WHITE_BISHOP = 4,
  BLACK_BISHOP = 5,
  WHITE_ROOK = 6,
  BLACK_ROOK = 7,
  WHITE_QUEEN = 8,
  BLACK_QUEEN = 9,
  WHITE_KING = 10,
  BLACK_KING = 11,

  PAWN = 0,
  KNIGHT = 1,
  BISHOP = 2,
  ROOK = 3,
  QUEEN = 4,
  KING = 5,

  NO_PIECE = 12
};

// 16 bit structure
// First 6 bits denote the origin square
// Next 6 bits denote the destination square
// Last 4 bits describe the move type
struct Move {
  uint16_t move_data;

  Move() : move_data(0) {};

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

struct UndoInfo {
  Move move;
  uint8_t captured_piece_type;
  uint8_t castling_rights; // 4 bits: white: king and queen side, black: king and queen side
  uint8_t en_passant_sq;
};

class Position{
public:
  std::array<uint64_t, 12> all_piece_bitboards;
  uint64_t white_bb;
  uint64_t black_bb;
  uint64_t total_bb;

  uint8_t c_rights;
  uint8_t en_passant_sq;
  bool side_to_move;
  uint16_t ply;

  std::array<UndoInfo, 2048> history_stack;

  std::array<uint8_t, 64> piece_list;

  Position() {
    all_piece_bitboards[WHITE_PAWN] = 0xFF00ULL;
    all_piece_bitboards[WHITE_KNIGHT] = 0x42ULL;
    all_piece_bitboards[WHITE_BISHOP] = 0x24ULL;
    all_piece_bitboards[WHITE_ROOK] = 0x81ULL;
    all_piece_bitboards[WHITE_KING] = 0x10ULL;
    all_piece_bitboards[WHITE_QUEEN] = 0x8ULL;

    all_piece_bitboards[BLACK_PAWN] = 0xFF'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_KNIGHT] = 0x42'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_BISHOP] = 0x24'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_ROOK] = 0x81'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_KING] = 0x10'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_QUEEN] = 0x8'00'00'00'00'00'00'00ULL;

    side_to_move = 0;
    ply = 0;
    c_rights = 0xF;
    en_passant_sq = 64;

    // PAWNS = 0/0b0    0
    // KNIGHT = 1/0b1   2
    // BISHOP = 2/0b10  4
    // ROOK = 3/0b11    6
    // QUEEN = 4/0b100  8
    // KING = 5/0b101   10
    // ADD ONE IF BLACK

    white_bb = 0;
    black_bb = 0;
    total_bb = 0;

    for (int i = 0; i<=10; i+=2){
      white_bb |= all_piece_bitboards[i];
      black_bb |= all_piece_bitboards[i+1];
    }

    total_bb = white_bb | black_bb;


    // Set up piece lists
    for (int i = 0; i<64; i++) {
      piece_list[i] = NO_PIECE;
    }

    // PAWNS 
    for (int i = 8; i<16; i++) {
      // WHITE 0b00
      piece_list[i] = WHITE_PAWN;
      // BLACK 0b00
      piece_list[i+40] = BLACK_PAWN;
    }

    // WHITE KNIGHTS 0b10
    piece_list[1] = WHITE_KNIGHT;
    piece_list[6] = WHITE_KNIGHT;

    // BLACK KNIGHTS 0b11
    piece_list[57] = BLACK_KNIGHT;
    piece_list[62] = BLACK_KNIGHT;

    // WHITE BISHOPS
    piece_list[2] = WHITE_BISHOP;
    piece_list[5] = WHITE_BISHOP;

    // BLACK BISHOPS
    piece_list[58] = BLACK_BISHOP;
    piece_list[61] = BLACK_BISHOP;

    // WHITE ROOKS
    piece_list[0] = WHITE_ROOK;
    piece_list[7] = WHITE_ROOK;

    // BLACK ROOKS
    piece_list[56] = BLACK_ROOK;
    piece_list[63] = BLACK_ROOK;

    // WHITE QUEEN
    piece_list[3] = WHITE_QUEEN;

    // BLACK QUEEN
    piece_list[59] = BLACK_QUEEN;

    // WHITE KING
    piece_list[4] = WHITE_KING;

    // BLACK KING
    piece_list[60] = BLACK_KING;
  }
  
  // Updates all relevant bitboards according to move
  // Assumes legal move
  void make_move(Move move){
    uint8_t from_sq = move.get_from_sq();
    uint8_t to_sq = move.get_to_sq();
    uint8_t moving_piece_type = piece_list[from_sq];
    uint8_t captured_piece_type = piece_list[to_sq];
    uint64_t from_bit = 1ULL << from_sq;
    uint64_t to_bit = 1ULL << to_sq;
    uint64_t move_mask = from_bit | to_bit;
    // UPDATE:
    // moving PIECE BITBOARD
    // CAPTURED PIECE BITBOARD (IF APPLICABLE)
    // OCCUPANCY BITBOARD

    history_stack[ply].castling_rights = c_rights;
    history_stack[ply].move = move;
    history_stack[ply].captured_piece_type = captured_piece_type;
    history_stack[ply].en_passant_sq = en_passant_sq;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    if (moving_piece_type & 1) {
      black_bb ^= move_mask;
    } else {
      white_bb ^= move_mask;
    }

    // Remove Captured Piece
    if (captured_piece_type < NO_PIECE) {
      all_piece_bitboards[captured_piece_type] ^= to_bit;
      if (captured_piece_type & 1) {
        black_bb ^= (to_bit);
      } else {
        white_bb ^= (to_bit);
      }
    }

    total_bb = black_bb & white_bb;

    // Update Piece Lists
    piece_list[from_sq] = NO_PIECE;
    piece_list[to_sq] = moving_piece_type;

    // Update other board state variables
    c_rights ^= MoveUtility::CASTLING_RIGHTS_UPDATE[from_sq];
    en_passant_sq = 64;
    if (moving_piece_type >> 1 == PAWN) {
      int8_t diff = to_sq - from_sq;
      if (diff == 16) {
        en_passant_sq = from_sq + 8;
      } else if (diff == -16) {
        en_passant_sq = from_sq - 8;
      }
    }

    ply++;
  }

  void unmake_move(UndoInfo unmove) {
    uint8_t from_sq = unmove.move.get_from_sq();
    uint8_t to_sq = unmove.move.get_to_sq();
    uint64_t from_bit = 1ULL << from_sq;
    uint64_t to_bit = 1ULL << to_sq;
    uint64_t move_mask = from_bit | to_bit;

    uint8_t moving_piece_type = piece_list[to_sq];

    c_rights = unmove.castling_rights;
    en_passant_sq = unmove.en_passant_sq;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    if (moving_piece_type & 1) {
      black_bb ^= move_mask;
    } else {
      white_bb ^= move_mask;
    }

    // Restore captured piece
    if (unmove.captured_piece_type < NO_PIECE) {
      all_piece_bitboards[unmove.captured_piece_type] ^= to_bit;
      if (unmove.captured_piece_type & 1) {
        black_bb ^= to_bit;
      } else {
        white_bb ^= to_bit;
      }
    }

    total_bb = white_bb & black_bb;

    // Update piece lists;
    piece_list[from_sq] = moving_piece_type;
    piece_list[to_sq] = unmove.captured_piece_type;

    ply--;
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
  Position test{};
  Move move(10, 18);
  test.make_move(move);
  std::cout << (int)test.en_passant_sq << '\n';
  return 0;
}
