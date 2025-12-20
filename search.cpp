#include "move.h"
#include "move_utility.h"
#include "piece.h"
#include "position.h"
#include "evaluation.h"
#include "move_generator.h"
#include <cstdint>
#include <iostream>
#include <string>

std::string move_to_string(const Move& move) {
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

Move string_to_move(std::string move_str, Position& pos) {

  MoveGenerator mg;
  mg.generate(pos);

  for (int i = 0; i < mg.count; i++) {
    Move legal_move = mg.move_list[i];

    if (move_str == move_to_string(legal_move)) {
      return legal_move;
    }
  }

  return Move();
}

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

int main() {
  std::string fen_string;
  std::cout << "Please enter fen string: ";
  std::getline(std::cin, fen_string);

  if (fen_string.size() < 4) fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  Position pos(fen_string);
  Move best_move;
  char user_side;
  std::string depth_str;
  int depth;

  bool wrong_char = true;

  std::cout << "Please enter depth: ";
  std::cin >> depth_str;
  depth = std::stoi(depth_str);

  while (true) {
    std::cout << "Please enter the side you will be playing as (w/b): ";
    std::cin >> user_side;
    if (user_side == 'b') {
      best_move = negamax_root(pos, depth);
      pos.make_move(best_move);
      std::cout << "My move: " << move_to_string(best_move) << std::endl;
      break;
    } else if (user_side == 'w') {
      break;
    }
  }
  
  while (true) {
    std::string user_move_str;
    std::cout << "Please enter your move: ";
    std::cin >> user_move_str;
    Move user_move = string_to_move(user_move_str, pos);
    if (user_move.move_data != 0) {

      pos.make_move(user_move);
      best_move = negamax_root(pos, depth);
      pos.make_move(best_move);
      std::cout << "My move: " << move_to_string(best_move) << std::endl;

    } else {
      std::cout << "Not a legal move" << std::endl;
      continue;
    }
  }

  return 0;
}