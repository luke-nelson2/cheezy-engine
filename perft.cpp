#include <cstdint>
#include <iostream>
#include <chrono>
#include "move_generator.h"
#include "move_utility.h"
#include "position.h"

std::string move_string(const Move& move) {
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

uint64_t perft(Position& pos, uint8_t depth) {

  if (depth == 0) return 1;

  uint64_t nodes = 0;
  MoveGenerator move_gen;

  move_gen.generate(pos);

  for (int i = 0; i < move_gen.count; i++) {
    pos.make_move(move_gen.move_list[i]);
    uint8_t king_square = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);
    if (move_gen.is_square_attacked(pos, king_square, pos.side_to_move ^ 1)) {
      pos.unmake_move();
      continue;
    }
    nodes += perft(pos,depth-1);
    pos.unmake_move();
  }

  return nodes;
}

void divide(Position& pos, uint8_t depth) {
    MoveGenerator move_gen;
    move_gen.generate(pos);

    uint64_t total = 0;

    for (int i = 0; i < move_gen.count; i++) {
        Move move = move_gen.move_list[i];
        
        pos.make_move(move);
        
        // Legality Check
        uint8_t king_sq = get_lsbit_index(pos.all_piece_bitboards[BLACK_KING - pos.side_to_move]);
        if (move_gen.is_square_attacked(pos, king_sq, pos.side_to_move ^ 1)) {
            pos.unmake_move();
            continue;
        }

        // Recursively count nodes for this branch
        uint64_t branch_nodes = perft(pos, depth - 1);
        total += branch_nodes;

        // Print the move and its count
        std::cout << move_string(move)
                  << ": " << branch_nodes << std::endl;

        pos.unmake_move();
    }
    
    std::cout << "\nTotal: " << total << std::endl;
}
      // Mine : Theirs
// e5c6: 42   : 41
// e5g6: 43   : 42
// e5d7: 46   : 45
// e5f7: 45   : 44
// 

int main() {
  std::string fen_string = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
  Position pos;

  auto start = std::chrono::high_resolution_clock::now();

  uint64_t nodes = perft(pos, 6);

  // End timing
  auto end = std::chrono::high_resolution_clock::now();

  // Calculate duration in seconds
  std::chrono::duration<double> duration = end - start;
  double seconds = duration.count();

  std::cout << "Time taken: " << seconds << " seconds" << std::endl;

  // Output the result of x / time
  if (seconds > 0) {
    double result = nodes / seconds;
    std::cout << "Result (x / time): " << result << std::endl;
  } else {
    std::cout << "Function was too fast to measure accurately." << std::endl;
  }

  // std::cout << nodes << std::endl;
  return 0;
}