#include <cstdint>
#include <iostream>

class Bitboard{
  uint64_t white_pawn;
  uint64_t white_knight;
  uint64_t white_bishop;
  uint64_t white_rook;
  uint64_t white_queen;
  uint64_t white_king;

  uint64_t black_pawn;
  uint64_t black_knight;
  uint64_t black_bishop;
  uint64_t black_rook;
  uint64_t black_queen;
  uint64_t black_king;
};

class Move{
public:
  uint16_t move_value;
private:
  uint64_t Knight_Attacks[64];

};

// Init bitboards
// Generate all possible moves
// How to store moves?
// minimax search with alpha beta pruning

int main(){
  return 0;
}
