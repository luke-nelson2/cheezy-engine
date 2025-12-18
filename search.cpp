#include "move.h"
#include "move_utility.h"
#include "position.h"
#include "evaluation.h"
#include "move_generator.h"
#include <cstdint>
#include <iostream>

std::string move_to_string(const Move& move) {
  std::string move_str = "";
  uint8_t from_sq = move.get_from_sq();
  uint8_t to_sq = move.get_to_sq();

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


  return move_str;
}

// Move string_to_move(std::string move_str) {

// }

// prioritze faster mate
// alpha beta pruning
// handle mates and draws
int32_t negamax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta) {
  if (depth == 0) return Evaluation::evaluate_position(pos);

  int32_t best_score = -40'000;
  int32_t score = 0;

  MoveGenerator move_gen;
  move_gen.generate(pos);

  for (int i = 0; i < move_gen.count; i++) {

    pos.make_move(move_gen.move_list[i]);
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      continue;
    }

    score = -negamax(pos, depth - 1, -beta, -alpha);
    if (score > best_score) {
      best_score = score;
    }

    if (best_score > alpha) {
      alpha = best_score;
    }

    if (alpha > beta) {
      pos.unmake_move();
      break;
    }
    pos.unmake_move();
  }

  return best_score;
}

Move negamax_root(Position& pos, uint8_t depth) {

  int32_t best_score = -40'000;
  Move best_move;
  int32_t score = 0;
  int32_t alpha = -40'000;
  int32_t beta = 40'000;

  MoveGenerator move_gen;
  move_gen.generate(pos);

  for (int i = 0; i < move_gen.count; i++) {

    pos.make_move(move_gen.move_list[i]);
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);

    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move^1)) {
      pos.unmake_move();
      continue;
    }

    score = -negamax(pos, depth - 1, -beta, -alpha);
    if (score > best_score) {
      best_score = score;
      best_move = move_gen.move_list[i];
    }

    if (best_score > alpha) {
      alpha = best_score;
    }

    if (alpha > beta) {
      pos.unmake_move();
      break;
    }

    pos.unmake_move();
  }

  return best_move;

}

int main() {
  std::string fen_string = "r2qkb1r/ppp1ppp1/2n2nb1/2P4p/4P3/PPNN1P2/3P2PP/R1BQKB1R w KQkq - 1 12";
  Position pos(fen_string);
  Move best_move = negamax_root(pos, 7);
  std::string move_str = move_to_string(best_move);
  std::cout << move_str << std::endl;
  return 0;
}