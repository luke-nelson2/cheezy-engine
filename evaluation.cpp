#include "move_utility.h"
#include "position.h"
#include <cstdint>

namespace Evaluation {

const std::array<uint8_t, 64> TABLE_IDX = {
  56, 57, 58, 59, 60, 61, 62, 63,
  48, 49, 50, 51, 52, 53, 54, 55,
  40, 41, 42, 43, 44, 45, 46, 47,
  32, 33, 34, 35, 36, 37, 38, 39,
  24, 25, 26, 27, 28, 29, 30, 31,
  16, 17, 18, 19, 20, 21, 22, 23,
   8,  9, 10, 11, 12, 13, 14, 15,
   0,  1,  2,  3,  4,  5,  6,  7,
};

const std::array<std::array<int, 64>, 7> PIECE_SQUARE_TABLES = {{

  // PAWNS
  {
    0,  0,  0,  0,  0,  0,  0,  0,
   50, 50, 50, 50, 50, 50, 50, 50,
   10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
  },

  // KNIGHTS
  {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
  },

  // BISHOPS
  {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
  },

  // ROOKS
  {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
  },

  // QUEENS
  {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
  },

  // KING MIDDLEGAME
  {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
  },

  // KING ENDGAME
  {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
  }
}};

const std::array<int, 5> PIECE_VALUES = {100, 350, 350, 525, 1000};

const int MATE_SCORE = 30000;
const int DRAW_SCORE = 0;

// 4 situations
// white is side to move: white pieces evald, black pieces evald
//   if white pieces are evald they should be positive
//   black pieces negative

// black is side to move: white pieces evald, black pieces evald
//   if white pieces are evald they should be negative
//   black pieces positive

// problem: side_to_move changes in the search
//   need a variable to represent who is moving at the TOP of the search tree
int32_t evaluation_position(const Position& pos) {

  int32_t score = 0;

  for (uint8_t piece = WHITE_PAWN; piece < WHITE_KING; piece++) {
    // array and indexing?
    uint8_t piece_type = piece >> 1;
    uint8_t piece_side = piece & 1U;
    int32_t piece_value = PIECE_VALUES[piece_type];
    int32_t piece_count = MoveUtility::count_bits(pos.all_piece_bitboards[piece]);
    

    if (piece_side == pos.side_to_move) {
      score += piece_count * piece_value;
    } else {
      score -= piece_count * piece_value;
    }

    // Positional Bonuses
    // WHILE POP
    uint64_t piece_bb = pos.all_piece_bitboards[piece];
    while (piece_bb) {
      
      uint8_t square = MoveUtility::get_lsbit_index(piece_bb);
      pop_bit(piece_bb, square);
      uint8_t idx = MoveUtility::NO_SQUARE;

      if (piece_side == WHITE) {
        idx = TABLE_IDX[square];
      } else {
        idx = square;
      }

      if (piece_side == pos.side_to_move) {
        score += PIECE_SQUARE_TABLES[piece_type][idx];
      } else {
        score -= PIECE_SQUARE_TABLES[piece_type][idx];
      }
    }

  }

  return score;

}

}
