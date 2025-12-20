#include "evaluation.h"
#include "move_utility.h"
#include "piece.h"
#include <cstdint>

namespace Evaluation {

std::array<std::array<int, 64>, 12> init_tables(bool eg) {
  std::array<std::array<int, 64>, 12> table;
  uint8_t piece, sq;
  for (piece = WHITE_PAWN; piece < NO_PIECE; piece+=2) {
    uint8_t piece_type = piece >> 1;
    for (sq = 0; sq < MoveUtility::NO_SQUARE; sq++) {
      if (eg) {
        table[piece][sq] = EG_PIECE_VALUES[piece_type] + eg_piece_tables[piece_type][sq^56];
        table[piece+1][sq] = EG_PIECE_VALUES[piece_type] + eg_piece_tables[piece_type][sq]; 
      } else {
        table[piece][sq] = MG_PIECE_VALUES[piece_type] + mg_piece_tables[piece_type][sq^56]; 
        table[piece+1][sq] = MG_PIECE_VALUES[piece_type] + mg_piece_tables[piece_type][sq]; 
      }
    }
  }
  return table;
};

const std::array<std::array<int, 64>, 12> eg_table = init_tables(1);
const std::array<std::array<int, 64>, 12> mg_table = init_tables(0);

int32_t evaluate_position(const Position& pos) {

  int32_t mg_score = 0;
  int32_t eg_score = 0;
  int32_t game_phase = 0;

  // WHITE PIECES
  for (uint8_t piece = WHITE_PAWN; piece < BLACK_KING; piece+=2) {
    // array and indexing?

    // Positional Bonuses
    // WHILE POP
    uint64_t piece_bb = pos.all_piece_bitboards[piece];
    while (piece_bb) {
      
      uint8_t square = MoveUtility::get_lsbit_index(piece_bb);
      pop_bit(piece_bb, square);

      mg_score += mg_table[piece][square];
      eg_score += mg_table[piece][square];
      game_phase += GAME_PHASE_INCREMENT[piece];
    } 

  }

  // BLACK PIECES
  for (uint8_t piece = BLACK_PAWN; piece < NO_PIECE; piece+=2) {
    // array and indexing?

    // Positional Bonuses
    // WHILE POP
    uint64_t piece_bb = pos.all_piece_bitboards[piece];
    while (piece_bb) {
      
      uint8_t square = MoveUtility::get_lsbit_index(piece_bb);
      pop_bit(piece_bb, square);

      mg_score -= mg_table[piece][square];
      eg_score -= eg_table[piece][square];
      game_phase += GAME_PHASE_INCREMENT[piece];
    } 

  }

  int mg_phase = game_phase;
  if (mg_phase > 256) mg_phase = 256;
  int eg_phase = 256 - mg_phase;

  int32_t score = ((mg_score * mg_phase + eg_score * eg_phase) >> 8);

  if (pos.side_to_move == BLACK) score = -score;

  return score;

}

}
