#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include "position.h"
#include "search.h"
#include "move_generator.h"
#include <cmath>

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

Move string_to_move(std::string move_str, Position& pos) {

  MoveGenerator mg;
  std::array<Move, 2> null_killer = {Move()};
  std::array<std::array<int32_t, 64>, 12> null_history = {0};
  Move null_move;
  mg.generate(pos, null_killer, null_history, null_move);

  for (int i = 0; i < mg.count; i++) {
    Move legal_move = mg.move_list[i];

    if (move_str == move_to_string(legal_move)) {
      pos.make_move(legal_move);
      uint8_t king_sq = MoveUtility::get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);
      bool legal = !mg.is_square_attacked(pos, king_sq, pos.side_to_move ^ 1);
      pos.unmake_move();
      if (legal) return legal_move;
    }
  }

  return Move();
}

bool valid_fen(const std::array<std::string, 6>& fields) {
  int ranks = 1, squares = 0, white_kings = 0, black_kings = 0;
  for (char c : fields[0]) {
    if (c == '/') {
      if (squares != 8) return false;
      ranks++;
      squares = 0;
    } else if (c >= '1' && c <= '8') {
      squares += c - '0';
    } else if (std::string("pnbrqkPNBRQK").find(c) != std::string::npos) {
      squares++;
      white_kings += c == 'K';
      black_kings += c == 'k';
    } else {
      return false;
    }
    if (squares > 8 || ranks > 8) return false;
  }
  if (ranks != 8 || squares != 8 || white_kings != 1 || black_kings != 1) return false;
  if (fields[1] != "w" && fields[1] != "b") return false;
  if (fields[2] != "-" && fields[2].find_first_not_of("KQkq") != std::string::npos) return false;
  if (fields[3] != "-" && (fields[3].size() != 2 || fields[3][0] < 'a' ||
      fields[3][0] > 'h' || (fields[3][1] != '3' && fields[3][1] != '6'))) return false;
  for (int i = 4; i < 6; i++) {
    if (fields[i].find_first_not_of("0123456789") != std::string::npos) return false;
    std::istringstream number(fields[i]);
    unsigned value;
    if (!(number >> value) || value > (i == 4 ? 255U : 65535U) || (i == 5 && value == 0)) return false;
  }
  return true;
}

bool set_position(std::istringstream& command, Position& pos) {
  std::string token;
  if (!(command >> token)) return false;
  Position candidate;
  if (token == "fen") {
    std::array<std::string, 6> fields;
    std::string fen;
    for (std::string& field : fields) {
      if (!(command >> field)) return false;
      if (!fen.empty()) fen += ' ';
      fen += field;
    }
    if (fen.size() > 255 || !valid_fen(fields)) return false;
    candidate = Position(fen);
  } else if (token != "startpos") {
    return false;
  }
  if (command >> token) {
    if (token != "moves") return false;
    while (command >> token) {
      // Leave room in the undo stack for search moves.
      if (candidate.ply >= candidate.history_stack.size() - 64) return false;
      Move move = string_to_move(token, candidate);
      if (move.move_data == 0) return false;
      candidate.make_move(move);
    }
  }
  pos = candidate;
  return true;
}

int main() {
  Position pos;
  Search srch;
  std::thread search_thread;
  auto stop_search = [&]() {
    srch.request_stop();
    if (search_thread.joinable()) search_thread.join();
  };

  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream command(line);
    std::string token;
    if (!(command >> token)) continue;

    if (token == "uci") {
      srch.write_uci_line("id name cheezy-engine");
      srch.write_uci_line("id author luke-nelson2");
      srch.write_uci_line("uciok");
    } else if (token == "isready") {
      srch.write_uci_line("readyok");
    } else if (token == "ucinewgame") {
      stop_search();
      pos = Position();
    } else if (token == "position") {
      stop_search();
      if (!set_position(command, pos)) srch.write_uci_line("info string Invalid position command");
    } else if (token == "go") {
      int depth = Search::MAX_DEPTH;
      bool valid = true;
      while (command >> token) {
        if (token == "depth") {
          std::string value;
          if (!(command >> value)) {
            valid = false;
            break;
          }
          std::istringstream number(value);
          if (!(number >> depth) || !number.eof() || depth < 1 || depth > Search::MAX_DEPTH) {
            valid = false;
            break;
          }
        }
      }
      if (!valid) {
        srch.write_uci_line("info string Invalid depth (expected 1 to 63)");
        continue;
      }
      stop_search();
      srch.reset_stop();
      // Search a copy; the GUI sends subsequent positions.
      search_thread = std::thread([&, search_pos = pos, depth]() mutable {
        Move best_move = srch.iterative_deepening(search_pos, static_cast<uint8_t>(depth));
        srch.write_uci_line("bestmove " + move_to_string(best_move));
      });
    } else if (token == "stop") {
      stop_search();
    } else if (token == "quit") {
      break;
    }
  }
  stop_search();
  return 0;
}
