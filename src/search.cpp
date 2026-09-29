#include "move.h"
#include "move_utility.h"
#include "piece.h"
#include "position.h"
#include "evaluation.h"
#include "move_generator.h"
#include "search.h"
#include <cstdint>
#include <iostream>
#include <sstream>

namespace {
std::string move_to_string(const Move& move) {
  if (move.move_data == 0) return "0000";
  std::string move_str = "";
  uint8_t from_sq = move.get_from_sq();
  uint8_t to_sq = move.get_to_sq();
  uint8_t flags = move.get_flags();

  uint8_t from_file = from_sq % 8;
  uint8_t from_rank = from_sq / 8;

  uint8_t to_file = to_sq % 8;
  uint8_t to_rank = to_sq / 8;

  char from_file_chr = from_file + 'a';
  char from_rank_chr = from_rank + '1';

  char to_file_chr = to_file + 'a';
  char to_rank_chr = to_rank + '1';

  move_str += from_file_chr;
  move_str += from_rank_chr;
  move_str += to_file_chr;
  move_str += to_rank_chr;

  if (flags <= PROMO_QUEEN && flags >= PROMO_KNIGHT) {
    char promo_char;
    if (flags == PROMO_KNIGHT) promo_char = 'n';
    if (flags == PROMO_BISHOP) promo_char = 'b';
    if (flags == PROMO_ROOK) promo_char = 'r';
    if (flags == PROMO_QUEEN) promo_char = 'q';

    move_str += promo_char;
  }

  return move_str;
}
}


// MOVE ORDERING:
//   For each move generated, assign a mvv_lva score to the score_list
//   Search the move with the highest score
//   If no cutoff occurs, search the move with the second highest score and so on

using namespace MoveUtility;


Move Search::iterative_deepening(Position& pos, uint8_t max_depth) {
  previous_pv.count = 0;
  
  clear_history();
  clear_killers();

  // Use the first legal move if stopped before depth 1 finishes.
  Move best_move;
  MoveGenerator move_gen;
  move_gen.generate(pos, killer_heuristic[0], history_heuristic, Move());
  for (int i = 0; i < move_gen.count; i++) {
    Move move = move_gen.move_list[i];
    pos.make_move(move);
    uint8_t king_sq = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);
    bool legal = !move_gen.is_square_attacked(pos, king_sq, pos.side_to_move ^ 1);
    pos.unmake_move();
    if (legal) {
      best_move = move;
      break;
    }
  }

  total_nodes = 0;
  if (best_move.move_data == 0) return best_move;

  max_depth = std::min(max_depth, MAX_DEPTH);
  for (uint8_t depth = 1; depth <= max_depth; depth++) {
    if (stop_requested.load()) break;

    uint64_t starting_nodes = total_nodes;
    Move iteration_move = negamax_root(pos, depth);
    if (stop_requested.load()) break;
    best_move = iteration_move;

    previous_pv = pv_table[0];

    double ebf = std::pow((double)(total_nodes - starting_nodes), 1.0 / depth);

    std::ostringstream info;
    info << "info depth " << (int)depth
         << " nodes " << total_nodes
         << " pv";
    for (int i = 0; i < previous_pv.count; i++) {
      info << " " << move_to_string(previous_pv.moves[i]);
    }
    write_uci_line(info.str());
    std::ostringstream stats;
    stats << "info string Eff BF: " << std::fixed << std::setprecision(2) << ebf;
    write_uci_line(stats.str());

  }

  return best_move;

}

// prioritze faster mate
// alpha beta pruning
// handle mates and draws
int32_t Search::negamax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta, bool is_pv_line) {

  pv_table[rel_ply].count = 0;
  if (stop_requested.load()) return 0;

  if (depth == 0) return quiescence(pos, alpha, beta);
  total_nodes++;
  if (rel_ply >= MAX_DEPTH) return Evaluation::evaluate_position(pos);

  int32_t best_score = -INF;
  int32_t score = 0;
  uint8_t legal_moves = 0;

  Move hint = Move();
  if (is_pv_line && rel_ply < previous_pv.count) {
    hint = previous_pv.moves[rel_ply];
  }

  MoveGenerator move_gen;
  move_gen.generate(pos, killer_heuristic[rel_ply], history_heuristic, hint);

  for (int i = 0; i < move_gen.count; i++) {

    if (stop_requested.load()) return 0;
    int best_idx = i;
    for (int j = i + 1; j < move_gen.count; j++) {
      if (move_gen.score_list[j] > move_gen.score_list[best_idx]) best_idx = j;
    }
    std::swap(move_gen.move_list[i], move_gen.move_list[best_idx]);
    std::swap(move_gen.score_list[i], move_gen.score_list[best_idx]);

    Move move = move_gen.move_list[i];
    pos.make_move(move);
    rel_ply++;
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      rel_ply--;
      continue;
    }
    legal_moves++;

    bool child_is_pv = is_pv_line && (move == hint);

    score = -negamax(pos, depth - 1, -beta, -alpha, child_is_pv);
    pos.unmake_move();
    rel_ply--;
    if (stop_requested.load()) return 0;

    if (score > best_score) best_score = score;
    if (score > alpha) {
      alpha = score;

      pv_table[rel_ply].moves[0] = move;

      for (uint8_t j = 0; j < pv_table[rel_ply + 1].count; j++) {
        pv_table[rel_ply].moves[j+1] = pv_table[rel_ply+1].moves[j];
      }

      pv_table[rel_ply].count = pv_table[rel_ply+1].count + 1;
    }

    // Beta cutoff
    if (alpha >= beta) {

      if (pos.piece_list[move.get_to_sq()] == NO_PIECE) {
        uint8_t piece = pos.piece_list[move.get_from_sq()];
        uint8_t to_sq = move.get_to_sq();

        update_killers(rel_ply, move);
        history_heuristic[piece][to_sq] += (depth * depth);
      }

      break;
    }
  }

  if (legal_moves == 0) {
    uint8_t current_king_sq = get_lsbit_index(pos.all_piece_bitboards[WHITE_KING + pos.side_to_move]);
    if (move_gen.is_square_attacked(pos, current_king_sq, pos.side_to_move)) {
      // CHECKMATE
      return -MATE_SCORE + rel_ply;
    } else {
      return 0;
    }
  }

  return best_score;
}

int32_t Search::quiescence(Position& pos, int32_t alpha, int32_t beta) {
  if (stop_requested.load()) return 0;
  total_nodes++;
  // Limit long chains of check evasions.
  if (rel_ply >= MAX_DEPTH) return Evaluation::evaluate_position(pos);

  MoveGenerator move_gen;
  uint8_t king_sq = get_lsbit_index(pos.all_piece_bitboards[WHITE_KING + pos.side_to_move]);
  bool in_check = move_gen.is_square_attacked(pos, king_sq, pos.side_to_move);
  int32_t best_score = in_check ? -INF : Evaluation::evaluate_position(pos);
  if (!in_check && best_score > alpha) alpha = best_score;

  move_gen.generate(pos, killer_heuristic[rel_ply], history_heuristic, Move());
  int legal_moves = 0;
  for (int i = 0; i < move_gen.count; i++) {
    if (stop_requested.load()) return 0;
    int best_idx = i;
    for (int j = i + 1; j < move_gen.count; j++) {
      if (move_gen.score_list[j] > move_gen.score_list[best_idx]) best_idx = j;
    }
    std::swap(move_gen.move_list[i], move_gen.move_list[best_idx]);
    std::swap(move_gen.score_list[i], move_gen.score_list[best_idx]);

    Move move = move_gen.move_list[i];
    uint8_t flags = move.get_flags();
    bool tactical = pos.piece_list[move.get_to_sq()] != NO_PIECE || flags == EN_PASSANT ||
                    (flags >= PROMO_KNIGHT && flags <= PROMO_QUEEN);
    pos.make_move(move);
    king_sq = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);
    if (move_gen.is_square_attacked(pos, king_sq, pos.side_to_move ^ 1)) {
      pos.unmake_move();
      continue;
    }
    legal_moves++;
    // Verify at least one legal move before stand pat, to recognize stalemate.
    if (!in_check && (alpha >= beta || !tactical)) {
      pos.unmake_move();
      if (alpha >= beta) return best_score;
      continue;
    }

    rel_ply++;
    int32_t score = -quiescence(pos, -beta, -alpha);
    pos.unmake_move();
    rel_ply--;
    if (stop_requested.load()) return 0;

    if (score > best_score) best_score = score;
    if (score > alpha) alpha = score;
    if (alpha >= beta) return best_score;
  }

  if (legal_moves == 0) return in_check ? -MATE_SCORE + rel_ply : 0;
  return best_score;
}

Move Search::negamax_root(Position& pos, uint8_t depth) {

  int32_t best_score = -INF;
  Move best_move;
  int32_t score = 0;
  int32_t alpha = -INF;
  int32_t beta = INF;

  rel_ply = 0;

  pv_table[rel_ply].count = 0;

  Move hint = (previous_pv.count > 0) ? previous_pv.moves[0] : Move();

  MoveGenerator move_gen;
  move_gen.generate(pos, killer_heuristic[rel_ply], history_heuristic, hint);
  uint8_t legal_moves = 0;

  for (int i = 0; i < move_gen.count; i++) {

    if (stop_requested.load()) break;
    int best_idx = i;
    for (int j = i + 1; j < move_gen.count; j++) {
      if (move_gen.score_list[j] > move_gen.score_list[best_idx]) best_idx = j;
    }
    std::swap(move_gen.move_list[i], move_gen.move_list[best_idx]);
    std::swap(move_gen.score_list[i], move_gen.score_list[best_idx]);

    pos.make_move(move_gen.move_list[i]);
    rel_ply++;
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      rel_ply--;
      continue;
    }

    legal_moves++;

    bool child_is_pv = (move_gen.move_list[i] == hint);

    if (legal_moves == 1) {
      score = -negamax(pos, depth - 1, -beta, -alpha, child_is_pv);
    } else {
      score = -negamax(pos, depth - 1, -alpha - 1, -alpha, false);

      if (score > alpha && score < beta) {
        score = -negamax(pos, depth-1, -beta, -alpha, false);
      }
    }
    
    pos.unmake_move();
    rel_ply--;
    if (stop_requested.load()) break;
    if (score > best_score) {
      best_score = score;
      best_move = move_gen.move_list[i];
    }

    if (score > alpha) {
      alpha = score;

      pv_table[rel_ply].moves[0] = move_gen.move_list[i];

      for (uint8_t j = 0; j < pv_table[rel_ply + 1].count; j++) {
        pv_table[rel_ply].moves[j+1] = pv_table[rel_ply+1].moves[j];
      }

      // std::cout << (int)pv_table[rel_ply].count << std::endl;
      pv_table[rel_ply].count = pv_table[rel_ply+1].count + 1;
    }


    
  }

  return best_move;

}
