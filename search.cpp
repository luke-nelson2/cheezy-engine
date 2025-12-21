#include "move.h"
#include "move_utility.h"
#include "piece.h"
#include "position.h"
#include "evaluation.h"
#include "move_generator.h"
#include "search.h"
#include <cstdint>
#include <iostream>



// MOVE ORDERING:
//   For each move generated, assign a mvv_lva score to the score_list
//   Search the move with the highest score
//   If no cutoff occurs, search the move with the second highest score and so on


const int32_t MATE_SCORE = 50'000;
// prioritze faster mate
// alpha beta pruning
// handle mates and draws
int32_t negamax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta) {
  if (depth == 0) return Evaluation::evaluate_position(pos);

  int32_t best_score = -60'000;
  int32_t score = 0;
  uint8_t legal_moves = 0;

  MoveGenerator move_gen;
  move_gen.generate(pos);

  for (int i = 0; i < move_gen.count; i++) {

    uint8_t best_idx = i;
    for (uint8_t j = i + 1; j < move_gen.count; j++) {
      if (move_gen.score_list[j] > move_gen.score_list[best_idx]) best_idx = j;
    }
    std::swap(move_gen.move_list[i], move_gen.move_list[best_idx]);
    std::swap(move_gen.score_list[i], move_gen.score_list[best_idx]);

    pos.make_move(move_gen.move_list[i]);
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      continue;
    }
    legal_moves++;

    score = -negamax(pos, depth - 1, -beta, -alpha);
    pos.unmake_move();

    if (score > best_score) best_score = score;
    if (score > alpha) alpha = score;
    if (alpha >= beta) break;
  }

  if (legal_moves == 0) {
    uint8_t current_king_sq = get_lsbit_index(pos.all_piece_bitboards[WHITE_KING + pos.side_to_move]);
    if (move_gen.is_square_attacked(pos, current_king_sq, pos.side_to_move)) {
      // CHECKMATE
      return -MATE_SCORE - depth;
    } else {
      return 0;
    }
  }

  return best_score;
}

Move negamax_root(Position& pos, uint8_t depth) {

  const int32_t INF = 60000;
  int32_t best_score = -INF;
  Move best_move;
  int32_t score = 0;
  int32_t alpha = -INF;
  int32_t beta = INF;

  MoveGenerator move_gen;
  move_gen.generate(pos);
  uint8_t legal_moves = 0;

  for (int i = 0; i < move_gen.count; i++) {

    uint8_t best_idx = i;
    for (uint8_t j = i + 1; j < move_gen.count; j++) {
      if (move_gen.score_list[j] > move_gen.score_list[best_idx]) best_idx = j;
    }
    std::swap(move_gen.move_list[i], move_gen.move_list[best_idx]);
    std::swap(move_gen.score_list[i], move_gen.score_list[best_idx]);

    pos.make_move(move_gen.move_list[i]);
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      continue;
    }

    legal_moves++;

    score = -negamax(pos, depth - 1, -beta, -alpha);
    if (score > best_score) {
      best_score = score;
      best_move = move_gen.move_list[i];
    }

    if (best_score > alpha) {
      alpha = best_score;
    }


    pos.unmake_move();
  }

  if (legal_moves == 0) {
    uint8_t current_king_sq = get_lsbit_index(pos.all_piece_bitboards[WHITE_KING + pos.side_to_move]);
    if (move_gen.is_square_attacked(pos, current_king_sq, pos.side_to_move)) {
      // CHECKMATE
      std::cout << "Mated" << std::endl;
    } else {
      // STALEMATE
      std::cout << "Stalemate" << std::endl;
    }
  }

  return best_move;

}


