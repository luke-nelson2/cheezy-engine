#include "attacks.h"
#include <cstdint>

namespace Attacks {

  std::array<uint64_t, 64> init_knight_table(){
    int idx;
    std::array<uint64_t, 64> knight_attacks;

    for (int rank=0; rank<8; rank++){
      for (int file=0; file<8; file++){
        uint64_t mask{0x0};
        idx = file*8+rank;
        uint64_t piece_bit = 1ULL << idx;
        uint64_t target;

        // SSW
        if (rank>0 && file>1) {
          target = piece_bit >> 17;
          mask |= target;
        }

        // SSE
        if (rank<7 && file>1) {
          target = piece_bit >> 15;
          mask |= target;
        }

        // ESE
        if (rank<6 && file>0) {
          target = piece_bit >> 6;
          mask |= target;
        }

        // ENE
        if (rank<6 && file<7) {
          target = piece_bit << 10;
          mask |= target;
        }

        // NNE
        if (rank<7 && file<6) {
          target = piece_bit << 17;
          mask |= target;
        }

        // NNW
        if (rank>0 && file<6) {
          target = piece_bit << 15;
          mask |= target;
        }

        // WNW
        if (rank>1 && file<7) {
          target = piece_bit << 6;
          mask |= target;
        }

        // WSW
        if (rank>1 && file>0) {
          target = piece_bit >> 10;
          mask |= target;
        }


        knight_attacks[idx] = mask;
      }
    }
    return knight_attacks;
  }

  std::array<std::array<uint64_t, 64>, 2> init_pawn_attack_table(){
    int idx;
    std::array<std::array<uint64_t, 64>, 2> pawn_attacks;
    for (int j=0; j<8; j++){
      for (int i=0; i<8; i++){
        idx = j*8 + i;
        pawn_attacks[0][idx] = 0;
        pawn_attacks[1][idx] = 0;
        if (j>0 && j<7){
          if (i>0){
            pawn_attacks[0][idx]  += 1ULL << (idx + 7);
            pawn_attacks[1][idx]  += 1ULL << (idx - 9);
          }
          if (i<7){
            pawn_attacks[0][idx]  += 1ULL << (idx + 9);
            pawn_attacks[1][idx]  += 1ULL << (idx - 7);
          }
        }
      }
    }
    return pawn_attacks;
  }

  std::array<uint64_t, 64> init_king_table(){
    int idx;
    std::array<uint64_t, 64> king_attacks;
    for (int i=0; i<8; i++){
      for (int j=0; j<8; j++){
        idx = j*8 + i;
        king_attacks[idx] = 0;
        if (i>0){
          // WEST
          king_attacks[idx] += (1ULL << (idx - 1));
          if (j>0){
            // SOUTH WEST
            king_attacks[idx] += (1ULL << (idx - 9));
          }
          if (j<7){
            // NORTH WEST
            king_attacks[idx] += (1ULL << (idx + 7));
          }
        }
        if (i<7){
          // EAST
          king_attacks[idx] += (1ULL << (idx + 1));
          if (j>0){
            // SOUTH EAST
            king_attacks[idx] += (1ULL << (idx - 7));
          }
          if (j<7){
            // NORTH EAST
            king_attacks[idx] += (1ULL << (idx + 9));
          }
        }
        if (j>0){
          // SOUTH
          king_attacks[idx] += (1ULL << (idx - 8));
        }
        if (j<7){
          // NORTH
          king_attacks[idx] += (1ULL << (idx + 8));
        }
      }
    }
    return king_attacks;
  }

  const std::array<uint64_t, 64> KNIGHT_MOVES = init_knight_table();
  const std::array<uint64_t, 64> KING_MOVES = init_king_table();
  const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS = init_pawn_attack_table();
}