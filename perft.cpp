#include <cstdint>
#include <iostream>
#include "move_generator.h"
#include "move_utility.h"
#include "position.h"

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
        std::cout << (int)move.get_from_sq() << ", "
                  << (int)move.get_to_sq()
                  << ": " << branch_nodes << std::endl;

        pos.unmake_move();
    }
    
    std::cout << "\nTotal: " << total << std::endl;
}

int main() {
  Position pos;
  std::cout << perft(pos, 6) << std::endl;
  return 0;
}