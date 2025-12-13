#pragma once
#include <array>
#include <cstdint>
#include <cmath>
#include "piece.h"
#include "move.h"
#include "move_utility.h"

struct UndoInfo {
  Move move;
  uint8_t captured_piece_type;
  uint8_t castling_rights; // 4 bits: white: king and queen side, black: king and queen side
  uint8_t en_passant_sq;
};

class Position{
public:
  std::array<uint64_t, 12> all_piece_bitboards;
  std::array<uint64_t, 2> occupancy_bitboards;
  uint64_t total_bb;

  uint8_t castling_rights;
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
    castling_rights = 0xF;
    en_passant_sq = 64;

    // PAWNS = 0/0b0    0
    // KNIGHT = 1/0b1   2
    // BISHOP = 2/0b10  4
    // ROOK = 3/0b11    6
    // QUEEN = 4/0b100  8
    // KING = 5/0b101   10
    // ADD ONE IF BLACK

    occupancy_bitboards[0] = 0;
    occupancy_bitboards[1] = 0;
    total_bb = 0;

    for (int i = 0; i<=10; i+=2){
      occupancy_bitboards[0] |= all_piece_bitboards[i];
      occupancy_bitboards[1] |= all_piece_bitboards[i+1];
    }

    total_bb = occupancy_bitboards[0] | occupancy_bitboards[1];


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

    history_stack[ply].castling_rights = castling_rights;
    history_stack[ply].move = move;
    history_stack[ply].captured_piece_type = captured_piece_type;
    history_stack[ply].en_passant_sq = en_passant_sq;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    occupancy_bitboards[moving_piece_type & 1] ^= move_mask;

    // Remove Captured Piece
    if (captured_piece_type < NO_PIECE) {
      all_piece_bitboards[captured_piece_type] ^= to_bit;
      occupancy_bitboards[captured_piece_type & 1] ^= to_bit;
    }

    total_bb = occupancy_bitboards[0] | occupancy_bitboards[1];

    // Update Piece Lists
    piece_list[from_sq] = NO_PIECE;
    piece_list[to_sq] = moving_piece_type;

    // Update other board state variables
    castling_rights &= ~MoveUtility::CASTLING_RIGHTS_UPDATE[from_sq];
    castling_rights &= ~MoveUtility::CASTLING_RIGHTS_UPDATE[to_sq];

    en_passant_sq = 64;
    if (moving_piece_type >> 1 == PAWN) {
      if (std::abs((int)to_sq - (int)from_sq) == 16) {
        en_passant_sq = (from_sq + to_sq) >> 1;
      } 
    }

    ply++;
  }

  void unmake_move() {

    uint8_t from_sq = history_stack[ply-1].move.get_from_sq();
    uint8_t to_sq = history_stack[ply-1].move.get_to_sq();
    uint64_t from_bit = 1ULL << from_sq;
    uint64_t to_bit = 1ULL << to_sq;
    uint64_t move_mask = from_bit | to_bit;

    uint8_t moving_piece_type = piece_list[to_sq];

    castling_rights = history_stack[ply-1].castling_rights;
    en_passant_sq = history_stack[ply-1].en_passant_sq;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    occupancy_bitboards[moving_piece_type & 1] ^= move_mask;

    // Restore captured piece
    if (history_stack[ply-1].captured_piece_type < NO_PIECE) {
      all_piece_bitboards[history_stack[ply-1].captured_piece_type] ^= to_bit;
      occupancy_bitboards[history_stack[ply-1].captured_piece_type & 1] ^= to_bit;
    }

    total_bb = occupancy_bitboards[0] | occupancy_bitboards[1];

    // Update piece lists;
    piece_list[from_sq] = moving_piece_type;
    piece_list[to_sq] = history_stack[ply-1].captured_piece_type;

    ply--;
  }
};