#include "evaluation.h"
#include "piece.h"
#include <cstdint>

namespace Evaluation {

// 4 situations
// white is side to move: white pieces evald, black pieces evald
//   if white pieces are evald they should be positive
//   black pieces negative

// black is side to move: white pieces evald, black pieces evald
//   if white pieces are evald they should be negative
//   black pieces positive

// problem: side_to_move changes in the search
//   need a variable to represent who is moving at the TOP of the search tree
int32_t evaluate_position(const Position& pos) {

  int32_t score = 0;

  // WHITE PIECES
  for (uint8_t piece = WHITE_PAWN; piece < BLACK_KING; piece+=2) {
    // array and indexing?
    uint8_t piece_type = piece >> 1;
    uint8_t piece_side = piece & 1U;
    int32_t piece_value = PIECE_VALUES[piece_type];
    int32_t piece_count = MoveUtility::count_bits(pos.all_piece_bitboards[piece]);
    
    score += piece_count * piece_value;

    // Positional Bonuses
    // WHILE POP
    uint64_t piece_bb = pos.all_piece_bitboards[piece];
    while (piece_bb) {
      
      uint8_t square = MoveUtility::get_lsbit_index(piece_bb);
      pop_bit(piece_bb, square);
      uint8_t idx = square^56;

      score += PIECE_SQUARE_TABLES[piece_type][idx];
    } 

  }

  // BLACK PIECES
  for (uint8_t piece = BLACK_PAWN; piece < NO_PIECE; piece+=2) {
    // array and indexing?
    uint8_t piece_type = piece >> 1;
    uint8_t piece_side = piece & 1U;
    int32_t piece_value = PIECE_VALUES[piece_type];
    int32_t piece_count = MoveUtility::count_bits(pos.all_piece_bitboards[piece]);
    
    score -= piece_count * piece_value;

    // Positional Bonuses
    // WHILE POP
    uint64_t piece_bb = pos.all_piece_bitboards[piece];
    while (piece_bb) {
      
      uint8_t square = MoveUtility::get_lsbit_index(piece_bb);
      pop_bit(piece_bb, square);
      uint8_t idx = square;

      score -= PIECE_SQUARE_TABLES[piece_type][idx];
    } 

  }

  if (pos.side_to_move == BLACK) {
    score = -score;
  }

  return score;

}

}
