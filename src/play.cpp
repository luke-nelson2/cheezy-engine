#include <iostream>
#include <string>
#include "position.h"
#include "search.h"
#include "move_generator.h"
#include <cmath>

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
  std::array<Move, 2> null_killer = {Move()};
  std::array<std::array<int32_t, 64>, 12> null_history = {0};
  Move null_move;
  mg.generate(pos, null_killer, null_history, null_move);

  for (int i = 0; i < mg.count; i++) {
    Move legal_move = mg.move_list[i];

    if (move_str == move_to_string(legal_move)) {
      return legal_move;
    }
  }

  return Move();
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

  Search srch;

  while (true) {
    std::cout << "Please enter the side you will be playing as (w/b): ";
    std::cin >> user_side;
    user_side = (user_side == 'w') ? 0 : 1;
    if (user_side != pos.side_to_move) {
      best_move = srch.iterative_deepening(pos, depth);
      // srch.print_stats(depth);
      pos.make_move(best_move);
      std::cout << "My move: " << move_to_string(best_move) << std::endl;
      break;
    } else if (user_side == pos.side_to_move) {
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
      best_move = srch.iterative_deepening(pos, depth);
      // srch.print_stats(depth);
      pos.make_move(best_move);
      std::cout << "My move: " << move_to_string(best_move) << std::endl;

    } else {
      std::cout << "Not a legal move" << std::endl;
      continue;
    }
  }

  return 0;
}
