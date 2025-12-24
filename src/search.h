#pragma once
#include <cstdint>
#include <array>
#include "position.h"
#include "move.h"

using PST = std::array<std::array<int32_t, 64>, 12>;

class Search {

public:

  Move negamax_root(Position& pos, uint8_t depth);

private:

  // [piece_type][to_sq]
  std::array<std::array<int32_t, 64>, 12> history_heuristic = {0};
  // [ply][move]
  std::array<std::array<Move, 2>, 256> killer_heuristic = {Move()};

  const int32_t MATE_SCORE = 50'000;
  const int32_t INF = 60000;

  int32_t negamax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta);

  inline void update_killers(uint8_t ply, Move move) {
    killer_heuristic[ply][1] = killer_heuristic[ply][0];
    killer_heuristic[ply][0] = move;
  }

  inline void clear_killers() {
    for (int i = 0; i < 256; i++) {
      killer_heuristic[i][0] = Move();
      killer_heuristic[i][1] = Move();
    }
  }

  inline void clear_history() {
    for (int i = 0; i < 12; i++) {
      for (int j = 0; j < 64; j++) {
        history_heuristic[i][j] = 0;
      }
    }
  }
  
};